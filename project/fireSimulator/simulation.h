#pragma once
#include "worldClasses.h"


class Simulation {
public:
    virtual ~Simulation() = default;
    virtual void Initialize(std::vector<Tile*>& startingTiles) = 0;
    virtual void Update() = 0;
    virtual std::vector<Tile*> GetLastChangedTiles() const = 0;
    virtual bool HasEnded() const = 0;
    virtual void Reset() = 0;
    virtual std::vector<Tile*> GetProhibitedTiles() const = 0;
    virtual std::unordered_map<int, sf::Color> GetChangedTileColors() const = 0;

protected:
    //  Uses a binary search algorithm to find the probability in each discrete step given the total probability and certain number of steps with the same chance/probability which together should make the total probability.
    float GetStepProbability(float totalProbability, int updateSteps) {
        float lowerBound = 0;
        float upperBound = 1;
        float p;
        for (int i = 0; i < 100; ++i) {
            p = (lowerBound + upperBound) / 2;
            float calcTotalProbability = p * (1 - std::pow(1 - p, updateSteps)) / p;
            if (calcTotalProbability > totalProbability) {
                upperBound = p;
            } else {
                lowerBound = p;
            }
        }
        return (lowerBound + upperBound) / 2;
    }
};



class FireSpreadSimulation : public Simulation {
    int currentTime_;

    World& world_;
    std::vector<Tile*> burningTiles_; // Currently burning tiles
    std::vector<Tile*> prohibitedTiles_; // Tiles that are not allowed to be clicked or to be start the simulation on / Here: all water tiles
    std::unordered_map<int, std::vector<Tile*>> changesOverTime_; // Tracks changed tiles at each time step - update of simulation

public:
    explicit FireSpreadSimulation(World& world) : world_(world), currentTime_(0) {
        InitWorldParameters();
        SetProhibitedTiles();
    }

    //  Initializes global and tile-specific parameters relevant to fire spread, such as wind speed, direction, and fire-related properties of tiles.
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

    //  Identifies and records tiles that cannot be involved in the fire spread (e.g., water tiles).
    void SetProhibitedTiles() {
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                if (tile != nullptr && tile->GetMoisture() == 100) {
                    prohibitedTiles_.push_back(tile);
                }
            }
        }
    }

    // Returns the list of tiles that are prohibited from burning.
    std::vector<Tile*> GetProhibitedTiles() const {
        return prohibitedTiles_;
    }



    //  Sets up the simulation with specified starting tiles, marking them as burning.
    void Initialize(std::vector<Tile*>& startingTiles) override {
        currentTime_ = 0;
        changesOverTime_.clear();
        burningTiles_.clear();

        for (auto& tile : startingTiles) {
            auto isBurning = tile->GetParameter<bool>("isBurning");
            if (isBurning) { // isBurning parameter exists for that tile
                isBurning->SetValue(true); // Explicitly set starting tiles as burning
                changesOverTime_[currentTime_].push_back(tile);
                burningTiles_.push_back(tile);
            }
        }
    }

    // Advances the simulation by one time step, updating the state of burning tiles and spreading fire according to various factors.
    void Update() override {
        currentTime_++; // Advance simulation time

        std::vector<Tile*> nextBurningTiles;
        for (auto* tile : burningTiles_) {
            auto neighbors = world_.GetNeighborTiles(tile);
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

    // Returns whether the simulation is completed.
    bool HasEnded() const {
        return burningTiles_.empty();
    }

    // Reinitializes the simulation and world parameters to their original/initial states.
    void Reset() {
        currentTime_ = 0;
        changesOverTime_.clear();
        burningTiles_.clear();
        prohibitedTiles_.clear();

        world_.ResetParameters(); // Resets global parameters
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                tile->ResetParameters(); // Resets each tile's parameters
            }
        }
    }


    // Fetches the list of tiles whose state changed in the last update.
    std::vector<Tile*> GetLastChangedTiles() const override {
        auto it = changesOverTime_.find(currentTime_);
        if (it != changesOverTime_.end()) {
            return it->second;
        }
        return std::vector<Tile*>();
    }

    // Provides a mapping of tile indices to their corresponding colors based on their current state, aiding in visualization.
    std::unordered_map<int, sf::Color> GetChangedTileColors() const {
        std::unordered_map<int, sf::Color> tileColors;
        for (const auto& tile : GetLastChangedTiles()) {
            // Determine color based on tile properties
            sf::Color color = tile->GetParameter<bool>("isBurning")->GetValue()? sf::Color(255, 105, 105) : sf::Color(180, 50, 50);
            int tileIndex = tile->GetWidthPosition() * world_.TilesOnSide() + tile->GetDepthPosition();// Logic to get tile's index in the visualizer
            tileColors[tileIndex] = color;
        }
        return tileColors;
    }


    // Determines whether a target tile will ignite from a source tile based on the calculated spread probability, simulating randomness with a range comparison.
    bool TryIgniteTile(Tile* source, Tile* target) {
        float spreadProbability = CalculateFireSpreadProbability(source, target);
        return Random::Range(0.0f, 1.0f) < spreadProbability;
    }

    // Integrates various environmental and situational factors to compute the overall probability of fire spreading from one tile to another.
    float CalculateFireSpreadProbability(Tile* source, Tile* target) {
        float vegetationFactor = GetVegetationFactor(target->GetVegetation(), 1.0f);
        float moistureFactor = GetMoistureFactor(target->GetMoisture(), 1.0f);
        float windFactor = GetWindFactor(world_, source, target,1.0f);
        float slopeFactor = GetSlopeFactor(source, target, 1.0f);

        float combined = (vegetationFactor + slopeFactor) / 2;
        float adjustedProbability = combined * moistureFactor * windFactor;

        return GetStepProbability(adjustedProbability, source->GetParameter<int>("burnTime")->GetValue());
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

    // Helper methods for factor calculations
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

    // Helper methods for factor calculations
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
        float windEffect = 1.0f;
        if (directionDifference <= 45) { // If wind is blowing directly towards the target
            windEffect += windSpeed * 0.03;
        } else if (directionDifference <= 90) {
            windEffect += windSpeed * 0.015;
        }

        // Ensure wind effect does not exceed a maximum threshold
        windEffect = std::min(windEffect, 1.5f); // Cap wind effect to prevent unrealistic spread rates

        return windEffect * spreadFactor;
    }

    // Helper methods for factor calculations
    float GetSlopeFactor(Tile* source, Tile* target, float spreadFactor) {
        float slopeDifference = target->GetHeight() - source->GetHeight();
        if (slopeDifference >= 0) {
            return 0.35f * spreadFactor;
        } else {
            return 0.25f * spreadFactor;
        }
    }
};