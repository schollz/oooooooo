#ifndef PERLIN_H
#define PERLIN_H
#pragma once

#include <algorithm>
#include <ctime>
#include <random>

// Perlin noise implementation
class PerlinNoise {
 private:
  std::vector<int> p;

 public:
  PerlinNoise() {
    // Initialize the permutation vector with values 0-255
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    // Randomize the permutation vector using a seed
    std::random_device rd;
    std::mt19937 engine(rd());
    std::shuffle(p.begin(), p.end(), engine);

    // Duplicate the permutation vector
    p.insert(p.end(), p.begin(), p.end());
  }

  // Fade function as defined by Ken Perlin
  double fade(double t) const { return t * t * t * (t * (t * 6 - 15) + 10); }

  // Linear interpolation
  double lerp(double t, double a, double b) const { return a + t * (b - a); }

  // Gradient function
  double grad(int hash, double x, double y, double z) const {
    // Convert low 4 bits of hash into 12 gradient directions
    int h = hash & 15;
    double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  // Return a noise value in the range [-1, 1]
  double noise(double x, double y, double z = 0) const {
    // Find the unit cube that contains the point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    // Find relative x, y, z of point in cube
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Compute fade curves for each of x, y, z
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // Hash coordinates of the 8 cube corners
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    // And add blended results from 8 corners of cube
    return lerp(
        w,
        lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
             lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
        lerp(v,
             lerp(u, grad(p[AA + 1], x, y, z - 1),
                  grad(p[BA + 1], x - 1, y, z - 1)),
             lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                  grad(p[BB + 1], x - 1, y - 1, z - 1))));
  }
};

#endif  // PERLIN_H