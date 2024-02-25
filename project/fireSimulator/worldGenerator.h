#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib> // For rand()
#include <ctime> // For time()
#include "perlin.h"

// General template definition
template<typename T>
class Map {
public:
    std::vector<std::vector<T>> data;
    int width, depth;

    Map(int width, int depth, T defaultValue = T()) : width(width), depth(depth), data(width, std::vector<T>(depth, defaultValue)) {}

    void SetData(int x, int y, T value) {
        if (x >= 0 && x < width && y >= 0 && y < depth) {
            data[x][y] = value;
        }
    }

    T GetData(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < depth) {
            return data[x][y];
        }
        return T(); // Return default value if out of bounds
    }
};

// Partial specialization definition for float
template<>
class Map<float> {
public:
    std::vector<std::vector<float>> data;
    int width, depth;

    Map(int width, int depth, float defaultValue = 0.0f) : width(width), depth(depth), data(width, std::vector<float>(depth, defaultValue)) {}

    void SetData(int x, int y, float value) {
        if (x >= 0 && x < width && y >= 0 && y < depth) {
            data[x][y] = value;
        }
    }

    float GetData(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < depth) {
            return data[x][y];
        }
        return 0.0f; // Return default value if out of bounds
    }

    void Normalize() {
        if (width == 0 || depth == 0) return; // Avoid division by zero

        float minValue = std::numeric_limits<float>::max();
        float maxValue = std::numeric_limits<float>::lowest();

        // Find min and max values
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < depth; ++y) {
                minValue = std::min(minValue, data[x][y]);
                maxValue = std::max(maxValue, data[x][y]);
            }
        }

        // Avoid division by zero if all values are the same
        if (minValue == maxValue) return;

        // Scale the values between 0 and 1
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < depth; ++y) {
                data[x][y] = (data[x][y] - minValue) / (maxValue - minValue);
            }
        }
    }

    void Smooth(int iterations = 1) {
        for (int iter = 0; iter < iterations; ++iter) {
            std::vector<std::vector<float>> tempData = data; // Copy data for calculation purposes

            for (int x = 1; x < width - 1; ++x) {
                for (int y = 1; y < depth - 1; ++y) {
                    float avg = (
                                        data[x - 1][y - 1] + data[x][y - 1] + data[x + 1][y - 1] +
                                        data[x - 1][y] + data[x][y] + data[x + 1][y] +
                                        data[x - 1][y + 1] + data[x][y + 1] + data[x + 1][y + 1]
                                ) / 9.0f;

                    tempData[x][y] = avg; // Assign the average to the temporary data
                }
            }

            data = tempData; // Update the original data with smoothed values
        }
    }

    void Amplify(float factor) {
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < depth; ++y) {
                data[x][y] *= factor; // Multiply each cell by the factor
            }
        }
    }
};


class IMapGenerator {
public:
    virtual ~IMapGenerator() = default;
    // Potentially other general-purpose virtual functions
};

class FloatMapGenerator : public IMapGenerator {
public:
    virtual Map<float> Generate() = 0;
};

class IntMapGenerator : public IMapGenerator {
public:
    virtual Map<int> Generate() = 0;
};

class BoolMapGenerator : public IMapGenerator {
public:
    virtual Map<bool> Generate() = 0;
};

class VegMapGenerator : public IMapGenerator {
public:
    virtual Map<VegetationType> Generate() = 0;
};


class BaseTerrainGenerator : public FloatMapGenerator {
    int width, depth;
    int octaves;
    float persistence;
    float scale;

public:
    BaseTerrainGenerator(int width, int depth, int octaves = 5, float persistence = 0.4f, float scale = 5.0f) :
            width(width), depth(depth), octaves(octaves), persistence(persistence), scale(scale) {}

    Map<float> Generate() {
        Map<float> map(width, depth);

        float offsetX = Random::Range(0, 10000);
        float offsetY = Random::Range(0, 10000);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                float amplitude = 1.3f;
                float frequency = 1.1f;
                float noiseHeight = 0.2f;

                for (int i = 0; i < octaves; i++) {
                    float sampleX = (x + offsetX) / scale * frequency;
                    float sampleY = (y + offsetY) / scale * frequency;

                    float perlinValue = perlin(sampleX, sampleY);
                    noiseHeight += perlinValue * amplitude;

                    amplitude *= persistence;
                    frequency *= 2;
                }

                map.SetData(x, y, noiseHeight);
            }
        }

        // Normalize the map data between 0 and 1
        map.Normalize();

        return map;
    }
};

class LakeMapGenerator : public BoolMapGenerator {
    Map<float>& heightMap;
    float lakeThreshold;

public:
    LakeMapGenerator(Map<float>& heightMap, float lakeThreshold)
            : heightMap(heightMap), lakeThreshold(lakeThreshold) {}

    Map<bool> Generate() override {
        Map<bool> lakeMap(heightMap.width, heightMap.depth, false);

        for (int x = 0; x < heightMap.width; x++) {
            for (int y = 0; y < heightMap.depth; y++) {
                lakeMap.SetData(x, y, heightMap.GetData(x, y) < lakeThreshold);
            }
        }

        return lakeMap;
    }
};

class RiverMapGenerator : public BoolMapGenerator {
    Map<float>& heightMap;
    Map<bool>& lakeMap;
    int rivers;

public:
    RiverMapGenerator(Map<float>& heightMap, Map<bool>& lakeMap, int rivers)
            : heightMap(heightMap), lakeMap(lakeMap), rivers(rivers) {}

    Map<bool> Generate() override {
        Map<bool> riverMap(heightMap.width, heightMap.depth);

        // Simplified river generation logic
        for (int i = 0; i < rivers; i++) {
            int riverStartX = Random::Range(0, heightMap.width);
            int riverStartY = Random::Range(0, heightMap.depth);

            int x = riverStartX;
            int y = riverStartY;
            int direction = Random::Range(0, 4);

            while (x >= 0 && x < heightMap.width && y >= 0 && y < heightMap.depth) {
                riverMap.SetData(x, y, 1);

                // Move in a semi-random direction
                switch (direction) {
                    case 0: if ((float)rand() / RAND_MAX < 0.5f) x++; else y++; break;
                    case 1: if ((float)rand() / RAND_MAX < 0.5f) y--; else x++; break;
                    case 2: if ((float)rand() / RAND_MAX < 0.5f) x--; else y--; break;
                    case 3: if ((float)rand() / RAND_MAX < 0.5f) y++; else x--; break;
                }

                if (x < 0 || y < 0 || x >= heightMap.width || y >= heightMap.depth || lakeMap.GetData(x, y) == 1)
                    break;
            }
        }

        return riverMap;
    }
};

class MoistureMapGenerator : public IntMapGenerator {
private:
    Map<float>& heightMap;
    Map<bool>& lakeMap;
    Map<bool>& riverMap;

    int moistureRadius = 2;
    int maxMoisture = 100;

    void SpreadMoisture(int x, int y, Map<int>& moistureMap) {
        for (int dx = -moistureRadius; dx <= moistureRadius; dx++) {
            for (int dy = -moistureRadius; dy <= moistureRadius; dy++) {
                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < heightMap.width && ny >= 0 && ny < heightMap.depth) {
                    int distance = std::abs(dx) + std::abs(dy);

                    if (distance <= moistureRadius) {
                        int influence = maxMoisture - (distance * (maxMoisture / moistureRadius));
                        moistureMap.SetData(nx, ny, std::min(moistureMap.GetData(nx, ny) + influence, maxMoisture));
                    }
                }
            }
        }
    }

public:
    MoistureMapGenerator(Map<float>& heightMap, Map<bool>& lakeMap, Map<bool>& riverMap)
            : heightMap(heightMap), lakeMap(lakeMap), riverMap(riverMap) {}

    Map<int> Generate() override {
        Map<int> moistureMap(heightMap.width, heightMap.depth);

        float offsetX = static_cast<float>(rand()) / RAND_MAX * 10000;
        float offsetY = static_cast<float>(rand()) / RAND_MAX * 10000;

        for (int x = 0; x < heightMap.width; x++) {
            for (int y = 0; y < heightMap.depth; y++) {
                if (lakeMap.GetData(x, y) == true || riverMap.GetData(x, y) == true) {
                    moistureMap.SetData(x, y, maxMoisture);
                    SpreadMoisture(x, y, moistureMap);
                } else {
                    float noise = perlin((x + offsetX) / 10.0f, (y + offsetY) / 10.0f);
                    // Normalize noise from -1 to 1, to 0 to 1
                    float normalizedNoise = (noise + 1.0f) / 2.0f;
                    // Scale to 0 to 100 range
                    float scaledNoise = normalizedNoise * 100.0f;
                    // Clamp the value to ensure it's within 0 to 100
                    float clampedNoise = std::max(0.0f, std::min(scaledNoise, 100.0f));
                    moistureMap.SetData(x, y, static_cast<int>(clampedNoise));
                }
            }
        }

        return moistureMap;
    }
};

class VegetationMapGenerator : public VegMapGenerator {
private:
    Map<int>& moistureMap;

public:
    VegetationMapGenerator(Map<int>& moistureMap)
            : moistureMap(moistureMap) {}

    Map<VegetationType> Generate() override {
        Map<VegetationType> vegetationMap(moistureMap.width, moistureMap.depth, VegetationType::Grass);

        for (int x = 0; x < moistureMap.width; x++) {
            for (int y = 0; y < moistureMap.depth; y++) {
                int moisture = moistureMap.GetData(x, y);

                // Default vegetation is Grass, no need to set it again
                if (static_cast<float>(rand()) / RAND_MAX <= 0.85f) { // 85% probability
                    if (moisture < 30) {
                        vegetationMap.SetData(x, y, VegetationType::Sparse);
                    } else if (moisture < 50) {
                        // Grass is the default, no need to set
                    } else if (moisture < 70) {
                        vegetationMap.SetData(x, y, VegetationType::Forest);
                    } else {
                        vegetationMap.SetData(x, y, VegetationType::Swamp);
                    }
                }
            }
        }
        return vegetationMap;
    }
};




// Main Generator of the entire terrain using different generators

#include <chrono> // For std::chrono::system_clock
#include <memory> // For std::unique_ptr

class WorldGenerator {
public:
    int width;
    int depth;
    int rivers;
    float lakeThreshold;

    WorldGenerator(int width, int depth, float lakeThreshold, int rivers)
        : width(width), depth(depth), lakeThreshold(lakeThreshold), rivers(rivers) {
        Random::InitState(static_cast<unsigned int>(time(nullptr)));
        srand(std::chrono::system_clock::now().time_since_epoch().count()); // Seed the random number generator
    }

    std::unique_ptr<World> Generate() {
        BaseTerrainGenerator heightMapGenerator(width, depth);
        auto heightMap = heightMapGenerator.Generate();

        LakeMapGenerator lakeMapGenerator(heightMap, lakeThreshold);
        auto lakeMap = lakeMapGenerator.Generate();

        RiverMapGenerator riverMapGenerator(heightMap, lakeMap, rivers);
        auto riverMap = riverMapGenerator.Generate();

        MoistureMapGenerator moistureMapGenerator(heightMap, lakeMap, riverMap);
        auto moistureMap = moistureMapGenerator.Generate();

        VegetationMapGenerator vegetationMapGenerator(moistureMap);
        auto vegetationMap = vegetationMapGenerator.Generate();

        return GenerateWorldFromMaps(heightMap, moistureMap, vegetationMap);
    }

private:
    std::unique_ptr<World> GenerateWorldFromMaps(const Map<float>& heightMap, const Map<int>& moistureMap, const Map<VegetationType>& vegetationMap) {
        auto world = std::make_unique<World>(width, depth);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                float height = heightMap.GetData(x, y);
                int moisture = moistureMap.GetData(x, y);
                VegetationType vegetation = vegetationMap.GetData(x, y);

                if (moisture == 100) {
                    height = 0.01f; // Ensure low height for maximum moisture areas
                }

                // Create a new tile with the specified attributes
                auto currTile = std::make_unique<Tile>(height, moisture, vegetation, x, y);

                // Place the tile in the world at the correct position
                world->SetTileAt(x, y, new Tile(height, moisture, vegetation, x, y));
            }
        }
        return world;
    }
};




