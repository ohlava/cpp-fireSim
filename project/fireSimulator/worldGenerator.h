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

typedef struct {
    float x, y;
} vector2;

vector2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    vector2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    vector2 gradient = randomGradient(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx * gradient.x + dy * gradient.y);
}

float interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}


// Sample Perlin noise at coordinates x, y
float perlin(float x, float y) {

    // Determine grid cell corner coordinates
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Compute Interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Compute and interpolate top two corners
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    // Compute and interpolate bottom two corners
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    // Final step: interpolate between the two previously interpolated values, now in y
    float value = interpolate(ix0, ix1, sy);

    return value;
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


class BaseTerrainGenerator {
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