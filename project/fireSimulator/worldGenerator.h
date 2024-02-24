#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib> // For rand()
#include <ctime> // For time()

class Random {
public:
    static void InitState(unsigned int seed) {
        srand(seed);
    }

    static float Range(float min, float max) {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
};

float PerlinNoise(float x, float y) {
    // Placeholder for Perlin noise function
    // You should replace this with a real Perlin noise algorithm
    return std::fmod((x * y), 1.0f);
}

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
};

class BaseTerrainGenerator {
    int width, depth;
    int octaves;
    float persistence;
    float scale;

public:
    BaseTerrainGenerator(int width, int depth, int octaves = 5, float persistence = 0.5f, float scale = 20.0f) :
            width(width), depth(depth), octaves(octaves), persistence(persistence), scale(scale) {}

    Map Generate() {
        Map map(width, depth);

        float offsetX = Random::Range(0, 10000);
        float offsetY = Random::Range(0, 10000);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                float amplitude = 1.0f;
                float frequency = 1.0f;
                float noiseHeight = 0.0f;

                for (int i = 0; i < octaves; i++) {
                    float sampleX = (x + offsetX) / scale * frequency;
                    float sampleY = (y + offsetY) / scale * frequency;

                    float perlinValue = PerlinNoise(sampleX, sampleY) * 2 - 1;
                    noiseHeight += perlinValue * amplitude;

                    amplitude *= persistence;
                    frequency *= 2;
                }

                map.SetData(x, y, noiseHeight);
            }
        }

        // Normalize or process the map data further if needed
        return map;
    }
};
