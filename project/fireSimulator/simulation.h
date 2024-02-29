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
    std::unordered_map<int, std::vector<Tile*>> changesOverTime_; // Tracks changed tiles at each time step
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
        world_.AddParameter("windSpeed", std::make_shared<TypedParameter<float>>(5.0f, 0.0f, 100.0f));
        world_.AddParameter("windDirection", std::make_shared<TypedParameter<int>>(0, 0, 360));

        // Initialize fire-related parameters for all tiles in the world
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                if (tile != nullptr) { // Ensure the tile exists
                    tile->AddParameter("isBurning", std::make_shared<TypedParameter<bool>>(false, false, true));
                    tile->AddParameter("HasBurned", std::make_shared<TypedParameter<bool>>(false, false, true));
                    tile->AddParameter("burnTime", std::make_shared<TypedParameter<int>>(0, 0, 5));
                    tile->AddParameter("burningFor", std::make_shared<TypedParameter<int>>(0, 0, 5));
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
                if (!neighbor->GetParameter<bool>("isBurning")->GetValue() && TryIgniteTile(tile, neighbor)) {
                    nextBurningTiles.push_back(neighbor);
                    neighbor->GetParameter<bool>("isBurning")->SetValue(true);
                    changesOverTime_[currentTime_].push_back(neighbor);
                }
            }

            // Update burning duration
            auto burningFor = tile->GetParameter<int>("burningFor")->GetValue() + 1;
            if (burningFor >= tile->GetParameter<int>("burnTime")->GetValue()) {
                tile->GetParameter<bool>("isBurning")->SetValue(false);
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
            if (!tile->GetParameter<bool>("HasBurned") || !tile->GetParameter<int>("burnTime") ||
                !tile->GetParameter<int>("burningFor") || !tile->GetParameter<bool>("isBurning")) {
                std::cerr << "One or more tiles are missing required fire parameters." << std::endl;
                return false;
            }
        }

        return true; // All required parameters are present
    }


    bool TryIgniteTile(Tile* source, Tile* target) {
        float spreadProbability = CalculateFireSpreadProbability(source, target);
        // Simulate a random chance, assuming RandomChance() returns a float between 0.0f and 1.0f
        return RandomChance() < spreadProbability;
    }

    float CalculateFireSpreadProbability(Tile* source, Tile* target) {
        // Adapt your C# CalculateFireSpreadProbability logic here, including vegetation, moisture, wind, and slope calculations
        // Example:
        return 0.5f; // Placeholder probability
    }

    float RandomChance() {
        // Placeholder for random chance generation, replace with actual random generation
        return 0.4f; // Example fixed value, use std::uniform_real_distribution for actual randomness
    }

};