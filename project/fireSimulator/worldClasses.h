#pragma once
#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include <stdexcept>
#include <random>
#include <cmath> // for std::max


// Parameter class for changeable properties within the simulation
class Parameter {
public:
    virtual ~Parameter() = default;
    virtual void Reset() = 0;
};

// Template class for specific types of parameters of any simulation. Controlling value ranges and providing a mechanism to reset to initial values.
template<typename T>
class TypedParameter : public Parameter {
    T initialValue_;
    T value_;
    T minValue_;
    T maxValue_;

public:
    TypedParameter(T initialValue, T minValue, T maxValue)
            : initialValue_(initialValue), value_(initialValue), minValue_(minValue), maxValue_(maxValue) {}

    void SetValue(T value) {
        value_ = std::max(minValue_, std::min(maxValue_, value));
    }

    T GetValue() const {
        return value_;
    }

    virtual void Reset() override {
        value_ = initialValue_;
    }
};

template<typename T>
class TypedVectorParameter {
    std::vector<T> values_;
    T initialValue_;
    T minValue_;
    T maxValue_;

public:
    TypedVectorParameter(size_t size, T initialValue, T minValue, T maxValue)
            : values_(size, initialValue), initialValue_(initialValue), minValue_(minValue), maxValue_(maxValue) {}

    void SetValue(size_t index, T value) {
        if (index >= values_.size()) {
            throw std::out_of_range("Index out of range");
        }
        values_[index] = std::max(minValue_, std::min(maxValue_, value));
    }

    T GetValue(size_t index) const {
        if (index >= values_.size()) {
            throw std::out_of_range("Index out of range");
        }
        return values_[index];
    }

    void Reset() {
        std::fill(values_.begin(), values_.end(), initialValue_);
    }
};

// Container for managing parameters. Manages a collection of parameters, allowing adding and retrieving typed parameters.
class ParameterContainer {
    std::unordered_map<std::string, std::shared_ptr<Parameter>> singleParameters_;
    std::unordered_map<std::string, std::shared_ptr<void>> vectorParameters_;

public:
    void AddParameter(const std::string& name, std::shared_ptr<Parameter> parameter) {
        singleParameters_[name] = std::move(parameter);
    }

    template<typename T>
    std::shared_ptr<TypedParameter<T>> GetParameter(const std::string& name) {
        auto it = singleParameters_.find(name);
        if (it != singleParameters_.end()) {
            return std::dynamic_pointer_cast<TypedParameter<T>>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    void AddVectorParameter(const std::string& name, size_t size, T initialValue, T minValue, T maxValue) {
        auto param = std::make_shared<TypedVectorParameter<T>>(size, initialValue, minValue, maxValue);
        vectorParameters_[name] = param;
    }

    template<typename T>
    std::shared_ptr<TypedVectorParameter<T>> GetVectorParameter(const std::string& name) {
        auto it = vectorParameters_.find(name);
        if (it != vectorParameters_.end()) {
            return std::static_pointer_cast<TypedVectorParameter<T>>(it->second);
        }
        return nullptr;
    }

    void ResetParameters() {
        for (auto& [name, param] : singleParameters_) {
            param->Reset();
        }
    }
};



enum class VegetationType {
    Grass,
    Sparse,
    Forest,
    Swamp
};

// Tile class for World class grid. Represent a terrain tile with attributes like height, moisture, and vegetation type, plus some optional parameters in a specific context.
class Tile : public ParameterContainer {
public:
    Tile(float height, int moisture, VegetationType vegetation, int positionX, int positionY)
            : height_(height), moisture_(moisture), vegetation_(vegetation),
              widthPosition_(positionX), depthPosition_(positionY) {
    }

    // Getters and setters
    int GetWidthPosition() const { return widthPosition_; }
    // void SetWidthPosition(int position) { widthPosition_ = std::max(0, position); }

    int GetDepthPosition() const { return depthPosition_; }
    // void SetDepthPosition(int position) { depthPosition_ = std::max(0, position); }

    float GetHeight() const { return height_; }
    // void SetHeight(float height) { height_ = std::max(0.0f, height); }

    int GetMoisture() const { return moisture_; }
    // void SetMoisture(int moisture) { moisture_ = std::max(0, std::min(100, moisture)); }

    VegetationType GetVegetation() const { return vegetation_; }
    // void SetVegetation(VegetationType vegetation) { vegetation_ = vegetation; }


private:
    int widthPosition_, depthPosition_;
    float height_;
    int moisture_;
    VegetationType vegetation_;
};

// World-class - Contains a 2D grid of Tile pointers, managing the terrain layout.
class World : public ParameterContainer {
public:
    std::vector<std::vector<Tile*>> grid; // 2D grid of Tile pointers

    World(int width, int depth) : width_(std::max(0, width)), depth_(std::max(0, depth)) {
        grid.resize(width_);
        for (int i = 0; i < width_; ++i) {
            grid[i].resize(depth_, nullptr); // Initialize with null pointers or actual Tile objects
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
        return grid[x][y];
    }

    void SetTileAt(int x, int y, Tile* tile) {
        if (x < 0 || x >= width_ || y < 0 || y >= depth_) {
            throw std::out_of_range("Coordinates are out of the grid bounds.");
        }
        delete grid[x][y]; // Clean up the existing tile if necessary
        grid[x][y] = tile; // Place the new tile
    }

    // Method for accessing neighboring tiles.
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

    // Method for accessing neighboring tiles neighboring just by the edge of some tile.
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

    int TilesOnSide() const {
        if (width_ != depth_) {
            throw std::runtime_error("There could be a problem, sizes of sides are not the same");
        }
        return width_;
    }

    int GetWidth() const {
        return width_;
    }

    int GetDepth() const {
        return depth_;
    }

    std::size_t GetTileIndex(const Tile* tile) const {
        return static_cast<std::size_t>(tile->GetWidthPosition()) * GetDepth() + tile->GetDepthPosition();
    }

private:
    int width_, depth_;
};