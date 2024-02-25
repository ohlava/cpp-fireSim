#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib> // For rand()
#include <ctime> // For time()
#include "perlin.h"

class Map {
public:
    std::vector<std::vector<float>> data;
    int width, depth;

    Map(int width, int depth) : width(width), depth(depth), data(width, std::vector<float>(depth, 0.0f)) {}

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

class MapGenerator {
public:
    virtual Map Generate() = 0; // Pure virtual function for generating a map
};

class LakeMapGenerator : public MapGenerator {
    Map& heightMap;
    float lakeThreshold;

public:
    LakeMapGenerator(Map& heightMap, float lakeThreshold)
            : heightMap(heightMap), lakeThreshold(lakeThreshold) {}

    Map Generate() override {
        Map lakeMap(heightMap.width, heightMap.depth);

        for (int x = 0; x < heightMap.width; x++) {
            for (int y = 0; y < heightMap.depth; y++) {
                lakeMap.SetData(x, y, heightMap.GetData(x, y) < lakeThreshold ? 1.0 : 0.0);
            }
        }

        return lakeMap;
    }
};

class RiverMapGenerator : public MapGenerator {
    Map& heightMap;
    Map& lakeMap;
    int rivers;

public:
    RiverMapGenerator(Map& heightMap, Map& lakeMap, int rivers)
            : heightMap(heightMap), lakeMap(lakeMap), rivers(rivers) {}

    Map Generate() override {
        Map riverMap(heightMap.width, heightMap.depth);

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

class BaseTerrainGenerator : public MapGenerator {
    int width, depth;
    int octaves;
    float persistence;
    float scale;

public:
    BaseTerrainGenerator(int width, int depth, int octaves = 5, float persistence = 0.4f, float scale = 20.0f) :
            width(width), depth(depth), octaves(octaves), persistence(persistence), scale(scale) {}

    Map Generate() {
        Map map(width, depth);

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