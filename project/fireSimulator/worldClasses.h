#pragma once
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <random>
#include <cmath> // for std::max


enum class VegetationType {
    Grass,
    Sparse,
    Forest,
    Swamp
};

// Weather class
class Weather {
public:
    Weather(int windDirection, float windSpeed)
            : windDirection_(windDirection), windSpeed_(windSpeed) {}

    void Reset() {
        // Create a random number generator
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine

        // Create distribution for wind direction (0 to 359)
        std::uniform_int_distribution<> disDir(0, 359);
        windDirection_ = disDir(gen);

        // Create distribution for wind speed (0.0 to 15.0)
        std::uniform_real_distribution<> disSpeed(0.0, 15.0);
        windSpeed_ = disSpeed(gen);
    }

    int GetWindDirection() const { return windDirection_; }
    float GetWindSpeed() const { return windSpeed_; }

    void ChangeWindDirection(int newDirection) {
        // Ensure the wind direction stays within 0-359 degrees
        windDirection_ = ((newDirection % 360) + 360) % 360;
    }

    void ChangeWindSpeed(float newSpeed) {
        // Ensure the wind speed is within a reasonable range
        windSpeed_ = std::max(0.0f, std::min(100.0f, newSpeed));
    }

    void Update() {
        std::random_device rd;
        std::mt19937 gen(rd());

        // Randomly change the wind direction by +/- 15 degrees
        std::uniform_int_distribution<> disDirChange(-15, 15);
        int windDirectionChange = disDirChange(gen);
        ChangeWindDirection(windDirection_ + windDirectionChange);

        // Randomly change the wind strength by +/- 4 km/h
        std::uniform_real_distribution<> disSpeedChange(-4.0, 4.0);
        float windSpeedChange = disSpeedChange(gen);
        ChangeWindSpeed(windSpeed_ + windSpeedChange);
    }


private:
    int windDirection_; // in degrees
    float windSpeed_; // in km/h
};

// Tile class
class Tile {
public:
    Tile(float height, int moisture, VegetationType vegetation, int positionX, int positionY)
            : height_(height), moisture_(moisture), vegetation_(vegetation),
              widthPosition_(positionX), depthPosition_(positionY) {
    }

    void Reset() {
    }

    // Getters and setters
    int GetWidthPosition() const { return widthPosition_; }
    void SetWidthPosition(int position) { widthPosition_ = std::max(0, position); }

    int GetDepthPosition() const { return depthPosition_; }
    void SetDepthPosition(int position) { depthPosition_ = std::max(0, position); }

    float GetHeight() const { return height_; }
    void SetHeight(float height) { height_ = std::max(0.0f, height); }

    int GetMoisture() const { return moisture_; }
    void SetMoisture(int moisture) { moisture_ = std::max(0, std::min(100, moisture)); }

    VegetationType GetVegetation() const { return vegetation_; }
    void SetVegetation(VegetationType vegetation) { vegetation_ = vegetation; }

private:
    int widthPosition_, depthPosition_;
    float height_;
    int moisture_;
    VegetationType vegetation_;
};

// World-class
class World {
public:
    World(int width, int depth) : width_(std::max(0, width)), depth_(std::max(0, depth)), weather_(0, 0) {
        grid_.resize(width_);
        for (int i = 0; i < width_; ++i) {
            grid_[i].resize(depth_, nullptr); // Initialize with nullptrs or actual Tile objects
        }
    }

    void Reset() {
        weather_.Reset();
        for (auto& row : grid_) {
            for (auto& tile : row) {
                if (tile) tile->Reset();
            }
        }
    }

    std::tuple<int, int> GetTilesDistanceXY(Tile* tile1, Tile* tile2) {
        int xDiff = tile1->GetWidthPosition() - tile2->GetWidthPosition();
        int yDiff = tile1->GetDepthPosition() - tile2->GetDepthPosition();
        return {xDiff, yDiff};
    }

    Tile* GetTileAt(int x, int y) {
        if (x < 0 || x >= width_ || y < 0 || y >= depth_) {
            throw std::out_of_range("Coordinates are out of the grid bounds.");
        }
        return grid_[x][y];
    }

    void SetTileAt(int x, int y, Tile* tile) {
        if (x < 0 || x >= width_ || y < 0 || y >= depth_) {
            throw std::out_of_range("Coordinates are out of the grid bounds.");
        }
        delete grid_[x][y]; // Clean up the existing tile if necessary
        grid_[x][y] = tile; // Place the new tile
    }

    std::vector<Tile*> GetNeighborTiles(Tile* tile, int distance = 1) {
        std::vector<Tile*> neighbors;
        int x = tile->GetWidthPosition();
        int y = tile->GetDepthPosition();

        for (int i = -distance; i <= distance; i++) {
            for (int j = -distance; j <= distance; j++) {
                int nx = x + i;
                int ny = y + j;
                if ((nx != x || ny != y) && nx >= 0 && nx < width_ && ny >= 0 && ny < depth_) {
                    neighbors.push_back(GetTileAt(nx, ny));
                }
            }
        }
        return neighbors;
    }

    std::vector<Tile*> GetEdgeNeighborTiles(Tile* tile) {
        std::vector<Tile*> edgeNeighbors;
        int x = tile->GetWidthPosition();
        int y = tile->GetDepthPosition();

        if (x + 1 < width_) edgeNeighbors.push_back(GetTileAt(x + 1, y));
        if (x - 1 >= 0) edgeNeighbors.push_back(GetTileAt(x - 1, y));
        if (y + 1 < depth_) edgeNeighbors.push_back(GetTileAt(x, y + 1));
        if (y - 1 >= 0) edgeNeighbors.push_back(GetTileAt(x, y - 1));

        return edgeNeighbors;
    }

    void UpdateWeather() {
        weather_.Update();
    }

private:
    int width_, depth_;
    std::vector<std::vector<Tile*>> grid_; // 2D grid of Tile pointers
    Weather weather_;
};