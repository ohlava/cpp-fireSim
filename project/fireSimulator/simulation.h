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

public:
    FireSpreadSimulation(World& world) : world_(world), currentTime_(0) {
        InitWorldParameters();
    }

    void Initialize(std::vector<Tile*>& startingTiles) override {
        currentTime_ = 0;
        changesOverTime_.clear();

        for (auto& tile : startingTiles) {
            auto isBurning = tile->GetParameter<bool>("isBurning");
            if (isBurning) { // checks if the isBurning parameter exists for the tile
                isBurning->SetValue(true); // Explicitly set starting tiles as burning
                changesOverTime_[currentTime_].push_back(tile);
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
                    tile->AddParameter("burnTime", std::make_shared<TypedParameter<int>>(0, 0, 100)); // Example range
                    tile->AddParameter("burningFor", std::make_shared<TypedParameter<int>>(0, 0, 100)); // Example range
                }
            }
        }
    }

    void Update() override {
        std::vector<Tile*> currentChangedTiles;
        // Simulation logic here: Check each tile, update status, track changes

        // Iterate over all tiles, update their states based on the simulation logic


        if (!currentChangedTiles.empty()) {
            currentTime_++;
            changesOverTime_[currentTime_] = currentChangedTiles;
        }
    }

    std::vector<Tile*> GetChangedTiles() const override {
        auto it = changesOverTime_.find(currentTime_);
        if (it != changesOverTime_.end()) {
            return it->second;
        }
        return std::vector<Tile*>();
    }

    bool HasEnded() const {
        for (const auto& [time, tiles] : changesOverTime_) {
            for (const auto& tile : tiles) {
                auto isBurning = tile->GetParameter<bool>("isBurning");
                if (isBurning && isBurning->GetValue()) {
                    return false; // Simulation continues if any tile is still burning
                }
            }
        }
        return true; // No tiles are burning, simulation ends
    }

    void Reset() {
        // Reset world and tiles to their initial state
        world_.ResetParameters(); // Resets global parameters like wind
        for (auto& row : world_.grid) {
            for (auto& tile : row) {
                tile->ResetParameters(); // Resets each tile's parameters
            }
        }VerifyRequiredParameters
    }

    // Implementation of required parameters verification
    virtual bool (World& world, const std::vector<Tile*>& tiles) const override {
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
};