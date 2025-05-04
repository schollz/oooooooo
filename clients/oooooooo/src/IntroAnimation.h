// IntroAnimation.h
#ifndef INTRO_ANIMATION_H
#define INTRO_ANIMATION_H
#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class IntroAnimation {
 public:
  IntroAnimation();
  ~IntroAnimation();

  void Init(TTF_Font* font);  // Changed to accept font
  void Update();
  void Render(SDL_Renderer* renderer, int windowWidth, int windowHeight);

  bool isComplete() const { return animationComplete; }
  void Start();
  void Stop();

 private:
  struct TrailPoint {
    float x, y;
    float alpha;
    int pixelSize;
  };

  bool animationComplete;
  bool animationRunning;
  float animationTime;
  float animationDuration;

  // Circle animation properties
  float circleAngle;
  float circleRadius;
  float centerX, centerY;

  // Trailing effect
  std::vector<TrailPoint> trail;
  int maxTrailLength;

  // Visual parameters
  float rotationSpeed;
  SDL_Color dotColor;
  SDL_Color trailColor;
  int dotSize;

  // Font for text display
  TTF_Font* font;

  void updateTrail(float currentX, float currentY);
  void renderPixelatedDot(SDL_Renderer* renderer, float x, float y, int size,
                          SDL_Color color, float alpha);
  void renderCenterText(SDL_Renderer* renderer, int windowWidth,
                        int windowHeight,
                        float globalAlpha);  // Updated signature
};

#endif