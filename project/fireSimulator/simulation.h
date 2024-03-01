#pragma once
#include "worldClasses.h"


class Simulation {
public:
    virtual ~Simulation() = default;
    virtual void Initialize(std::vector<Tile*>& startingTiles) = 0;
    virtual void Update() = 0;
    virtual std::vector<Tile*> GetChangedTiles() const = 0;
    virtual bool HasEnded() const = 0;
    virtual void Reset() = 0;
    virtual bool VerifyRequiredParameters(World& world, const std::vector<Tile*>& tiles) const = 0;
};



class FireSpreadSimulation : public Simulation {
    World& world_;
    std::unordered_map<int, std::vector<Tile*>> changesOverTime_; // Tracks changed tiles at each time step - update of simulation
    int currentTime_;

    std::vector<Tile*> burningTiles_;

public:
    FireSpreadSimulation(World& world) : world_(world), currentTime_(0) {
        InitWorldParameters();
    }

    void Initialize(std::vector<Tile*>& startingTiles) override {
        currentTime_ = 0;
        changesOverTime_.clear();
        burningTiles_.clear();

        for (auto& tile : startingTiles) {
            auto isBurning = tile->GetParameter<bool>("isBurning");
            if (isBurning) { // checks if the isBurning parameter exists for the tile
                isBurning->SetValue(true); // Explicitly set starting tiles as burning
                changesOverTime_[currentTime_].push_back(tile);
                burningTiles_.push_back(tile);
            }
        }
    }

    void InitWorldParameters() {
        // Initialize global parameters in the world
        world_.AddParameter("windSpeed", std::make_shared<TypedParameter<float>>(5.0f, 0.0f, 50.0f));
        world_.AddParameter("windDirection", std::make_shared<TypedParameter<int>>(0, 0, 360));

        // Initialize fire-related parameters for all tiles in the world
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                if (tile != nullptr) { // Ensure the tile exists
                    tile->AddParameter("isBurning", std::make_shared<TypedParameter<bool>>(false, false, true));
                    tile->AddParameter("hasBurned", std::make_shared<TypedParameter<bool>>(false, false, true));
                    tile->AddParameter("burningFor", std::make_shared<TypedParameter<int>>(0, 0, 5));

                    // Determine burnTime based on vegetation type
                    int burnTime = 5; // Default burn time
                    switch (tile->GetVegetation()) {
                        case VegetationType::Grass:
                            burnTime = 1;
                            break;
                        case VegetationType::Sparse:
                            burnTime = 2;
                            break;
                        case VegetationType::Swamp:
                            burnTime = 3;
                            break;
                        case VegetationType::Forest:
                            burnTime = 4;
                            break;
                        default:
                            // Keep default value if no matching vegetation type
                            break;
                    }

                    // Add burnTime parameter to the tile with the determined value
                    tile->AddParameter("burnTime", std::make_shared<TypedParameter<int>>(burnTime, 0, 5));
                }
            }
        }
    }

    void Update() override {

        currentTime_++; // Advance simulation time

        std::vector<Tile*> nextBurningTiles;
        for (auto* tile : burningTiles_) {
            auto neighbors = world_.GetNeighborTiles(tile); // This function needs to be defined or adapted.
            for (auto* neighbor : neighbors) {
                if (!neighbor->GetParameter<bool>("isBurning")->GetValue() && !neighbor->GetParameter<bool>("hasBurned")->GetValue()
                        && TryIgniteTile(tile, neighbor)) {
                    // Neighbor tile catching on fire
                    nextBurningTiles.push_back(neighbor);
                    neighbor->GetParameter<bool>("isBurning")->SetValue(true);
                    changesOverTime_[currentTime_].push_back(neighbor);
                }
            }

            // Update burning duration
            auto burningFor = tile->GetParameter<int>("burningFor")->GetValue() + 1;
            if (burningFor >= tile->GetParameter<int>("burnTime")->GetValue()) {
                tile->GetParameter<bool>("isBurning")->SetValue(false);
                tile->GetParameter<bool>("hasBurned")->SetValue(true);
                changesOverTime_[currentTime_].push_back(tile);
            } else {
                tile->GetParameter<int>("burningFor")->SetValue(burningFor);
                nextBurningTiles.push_back(tile);
            }
        }

        burningTiles_ = std::move(nextBurningTiles); // Update the list of burning tiles for the next cycle
    }

    std::vector<Tile*> GetChangedTiles() const override {
        auto it = changesOverTime_.find(currentTime_);
        if (it != changesOverTime_.end()) {
            return it->second;
        }
        return std::vector<Tile*>();
    }


    std::unordered_map<int, sf::Color> GetChangedTileColors() const {
        std::unordered_map<int, sf::Color> tileColors;
        for (const auto& tile : GetChangedTiles()) {
            // Determine color based on tile properties
            sf::Color color = tile->GetParameter<bool>("isBurning")->GetValue()? sf::Color(255, 105, 105) : sf::Color(180, 50, 50);
            int tileIndex = tile->GetWidthPosition() * world_.TilesOnSide() + tile->GetDepthPosition();// Logic to get tile's index in the visualizer
            tileColors[tileIndex] = color;
        }
        return tileColors;
    }

    bool HasEnded() const {
        return burningTiles_.empty();
    }

    void Reset() {
        currentTime_ = 0;
        changesOverTime_.clear();
        burningTiles_.clear();

        // Reset world and tiles to their initial state
        world_.ResetParameters(); // Resets global parameters like wind
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                tile->ResetParameters(); // Resets each tile's parameters
            }
        }
    }

    // Implementation of required parameters verification
    virtual bool VerifyRequiredParameters(World& world, const std::vector<Tile*>& tiles) const override {
        // Verify world parameters
        if (!world.GetParameter<float>("windSpeed") || !world.GetParameter<int>("windDirection")) {
            std::cerr << "World is missing required wind parameters." << std::endl;
            return false;
        }

        // Verify tile parameters
        for (const auto& tile : tiles) {
            if (!tile->GetParameter<bool>("hasBurned") || !tile->GetParameter<int>("burnTime") ||
                !tile->GetParameter<int>("burningFor") || !tile->GetParameter<bool>("isBurning")) {
                std::cerr << "One or more tiles are missing required fire parameters." << std::endl;
                return false;
            }
        }

        return true; // All required parameters are present
    }

    // Helper methods for factor calculations
    float GetVegetationFactor(VegetationType vegetation, float spreadFactor) {
        float factor = 1.0f;
        switch (vegetation) {
            case VegetationType::Grass: factor = 0.18f; break;
            case VegetationType::Forest: factor = 0.4f; break;
            case VegetationType::Sparse: factor = 0.25f; break;
            case VegetationType::Swamp: factor = 0.22f; break;
        }
        return factor * spreadFactor;
    }

    float GetMoistureFactor(int moisture, float spreadFactor) {
        if (moisture == 100) {
            return 0; // water tile
        }

        if (moisture > 85) {
            return 0.5f;
        } else if (moisture > 65) {
            return 0.7f;
        } else {
            return 0.88f;
        }
    }

    float GetWindFactor(World& world, Tile* source, Tile* target, float spreadFactor) {
        auto windSpeed = world.GetParameter<float>("windSpeed")->GetValue();
        auto windDirection = world.GetParameter<int>("windDirection")->GetValue();

        // Calculate direction from source to target tile
        float deltaX = target->GetWidthPosition() - source->GetWidthPosition();
        float deltaY = target->GetDepthPosition() - source->GetDepthPosition();
        float angleToTarget = std::atan2(deltaY, deltaX) * (180 / M_PI);
        if (angleToTarget < 0) {
            angleToTarget += 360; // Normalize angle to 0-359 degrees
        }

        // Calculate difference between wind direction and direction to target
        float directionDifference = std::fabs(windDirection - angleToTarget);
        if (directionDifference > 180) {
            directionDifference = 360 - directionDifference; // Ensure shortest path angle
        }

        // Adjust spread factor based on wind direction and speed
        float windEffect = 1.0f; // Base wind effect
        if (directionDifference <= 45) { // If wind is blowing directly towards the target
            windEffect += windSpeed * 0.03; // Increase spread factor by a percentage of the wind speed
        } else if (directionDifference <= 90) {
            windEffect += windSpeed * 0.015; // Smaller increase if wind is not directly aligned
        }

        // Ensure wind effect does not exceed a maximum threshold
        windEffect = std::min(windEffect, 1.5f); // Cap wind effect to prevent unrealistic spread rates

        return windEffect * spreadFactor;
    }

    float GetSlopeFactor(Tile* source, Tile* target, float spreadFactor) {
        float slopeDifference = target->GetHeight() - source->GetHeight();
        if (slopeDifference >= 0) {
            return 0.35f * spreadFactor;
        } else {
            return 0.25f * spreadFactor;
        }
    }

    float CalculateFireSpreadProbability(Tile* source, Tile* target) {
        float vegetationFactor = GetVegetationFactor(target->GetVegetation(), 1.0f);
        float moistureFactor = GetMoistureFactor(target->GetMoisture(), 1.0f);
        float windFactor = GetWindFactor(world_, source, target,1.0f); // Add necessary arguments
        float slopeFactor = GetSlopeFactor(source, target, 1.0f);

        float combined = (vegetationFactor + slopeFactor) / 2;
        float adjustedProbability = combined * moistureFactor * windFactor;

        return GetStepFireProbability(adjustedProbability, source->GetParameter<int>("burnTime")->GetValue());
    }

    float GetStepFireProbability(float totalProbability, int BurnTime) {
        float lowerBound = 0;
        float upperBound = 1;
        float p;
        for (int i = 0; i < 100; ++i) {
            p = (lowerBound + upperBound) / 2;
            float calcTotalProbability = p * (1 - std::pow(1 - p, BurnTime)) / p;
            if (calcTotalProbability > totalProbability) {
                upperBound = p;
            } else {
                lowerBound = p;
            }
        }
        return (lowerBound + upperBound) / 2;
    }

    bool TryIgniteTile(Tile* source, Tile* target) {
        float spreadProbability = CalculateFireSpreadProbability(source, target);
        return Random::Range(0.0f, 1.0f) < spreadProbability;
    }

};