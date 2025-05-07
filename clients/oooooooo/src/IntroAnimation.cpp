// IntroAnimation.cpp
#include "IntroAnimation.h"

#include <algorithm>

IntroAnimation::IntroAnimation()
    : animationComplete(false),
      animationRunning(false),
      animationTime(0.0f),
      animationDuration(8.0f),  // 3 seconds animation
      circleAngle(0.8f),
      circleRadius(180.0f),  // Fixed radius
      centerX(0.0f),
      centerY(0.0f),
      maxTrailLength(380),
      rotationSpeed(1.5f),  // rotations per second
      dotSize(8),
      font(nullptr) {
  // Initialize colors
  dotColor = {255, 255, 255, 255};    // White
  trailColor = {180, 180, 180, 255};  // Light gray
}

IntroAnimation::~IntroAnimation() {
  trail.clear();
  font = nullptr;  // Don't delete, just clear the reference
}

void IntroAnimation::Init(TTF_Font* font) {
  this->font = font;
  trail.clear();
  animationTime = 0.0f;
  circleAngle = 0.0f;
  animationComplete = false;
  animationRunning = false;
}

void IntroAnimation::Start() {
  if (!animationRunning) {
    animationRunning = true;
  }
}

void IntroAnimation::Stop() {
  animationRunning = false;
  animationComplete = true;
}

void IntroAnimation::Update() {
  if (!animationRunning || animationComplete) return;

  // Update animation time (assuming 60 FPS)
  animationTime += 0.016f;

  if (animationTime >= animationDuration) {
    Stop();
    return;
  }

  // Update circle angle
  circleAngle +=
      rotationSpeed * 2.0f * M_PI * 0.016f;  // Rotate based on frame time

  // For a more interesting effect, vary the radius
  float radiusVariation = sin(animationTime * 2.0f) * 30.0f;
  float currentRadius = circleRadius + radiusVariation;

  // Calculate current position
  float currentX = centerX + currentRadius * cos(circleAngle);
  float currentY = centerY + currentRadius * sin(circleAngle);

  // Update trail
  updateTrail(currentX, currentY);
}

void IntroAnimation::updateTrail(float currentX, float currentY) {
  // Add new point to trail
  TrailPoint newPoint;
  newPoint.x = currentX;
  newPoint.y = currentY;
  newPoint.alpha = 1.0f;
  newPoint.pixelSize = 2;  // Start small

  trail.insert(trail.begin(), newPoint);

  // Update existing trail points
  for (size_t i = 1; i < trail.size(); ++i) {
    float age = static_cast<float>(i) / static_cast<float>(maxTrailLength);

    // Fade out
    trail[i].alpha = std::max(0.0f, 1.0f - age);

    // Increase pixelation with age
    trail[i].pixelSize = 2 + static_cast<int>(age * 6.0f);  // Up to 8x8 pixels
  }

  // Remove old trail points
  if (trail.size() > static_cast<size_t>(maxTrailLength)) {
    trail.resize(maxTrailLength);
  }
}
void IntroAnimation::Render(SDL_Renderer* renderer, int windowWidth,
                            int windowHeight) {
  if (!animationRunning || animationComplete) return;

  // Calculate global fade alpha
  float globalAlpha;

  // Fade in during first 0.5 seconds
  if (animationTime < 0.5f) {
    globalAlpha = animationTime * 2.0f;
  }
  // Stay fully visible until last 0.5 seconds
  else if (animationTime < animationDuration - 0.5f) {
    globalAlpha = 1.0f;
  }
  // Fade out during last 0.5 seconds
  else {
    float fadeOutProgress = (animationTime - (animationDuration - 0.5f)) / 0.5f;
    globalAlpha = 1.0f - fadeOutProgress;
  }

  // Update center position based on window size
  centerX = windowWidth / 2.0f;
  centerY = windowHeight / 2.0f;

  // Draw trail first (behind the main dot)
  for (size_t i = trail.size(); i > 0; --i) {
    const auto& point = trail[i - 1];
    // Apply both trail alpha and global fade
    float combinedAlpha = point.alpha * globalAlpha;
    renderPixelatedDot(renderer, point.x, point.y, point.pixelSize, trailColor,
                       combinedAlpha);
  }

  // Draw main dot (if we have trail points)
  if (!trail.empty()) {
    const auto& frontPoint = trail.front();
    // Apply global fade to main dot
    renderPixelatedDot(renderer, frontPoint.x, frontPoint.y, dotSize, dotColor,
                       globalAlpha);
  }

  // Render the center text with global fade
  renderCenterText(renderer, windowWidth, windowHeight, globalAlpha);
}

void IntroAnimation::renderPixelatedDot(SDL_Renderer* renderer, float x,
                                        float y, int size, SDL_Color color,
                                        float alpha) {
  // Apply alpha to color
  Uint8 adjustedAlpha = static_cast<Uint8>(color.a * alpha);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, adjustedAlpha);

  // Create a pixelated effect by drawing a square of the specified size
  SDL_Rect pixelRect = {static_cast<int>(x - size / 2.0f),
                        static_cast<int>(y - size / 2.0f), size, size};

  SDL_RenderFillRect(renderer, &pixelRect);
}
void IntroAnimation::renderCenterText(SDL_Renderer* renderer, int windowWidth,
                                      int windowHeight, float globalAlpha) {
  if (!font) return;

  // Use the global alpha directly
  Uint8 alpha = static_cast<Uint8>(globalAlpha * 255);
  SDL_Color textColor = {255, 255, 255, alpha};  // White with fade

  // Render multiple lines of text
  const char* subTexts[] = {"oooooooo", "", "digital tape loops x8",
                            "v0.1.3",   "", "h for help"};
  int numSubTexts = sizeof(subTexts) / sizeof(subTexts[0]);

  for (int i = 0; i < numSubTexts; ++i) {
    SDL_Surface* subTextSurface =
        TTF_RenderText_Blended(font, subTexts[i], textColor);
    if (subTextSurface) {
      SDL_Texture* subTextTexture =
          SDL_CreateTextureFromSurface(renderer, subTextSurface);
      if (subTextTexture) {
        int textWidth = subTextSurface->w;
        int textHeight = subTextSurface->h;
        SDL_Rect subTextRect = {windowWidth / 2 - textWidth / 2,
                                windowHeight / 2 + i * 20 - 65, textWidth,
                                textHeight};  // Move up by 50 pixels
        SDL_RenderCopy(renderer, subTextTexture, nullptr, &subTextRect);
        SDL_DestroyTexture(subTextTexture);
      }
      SDL_FreeSurface(subTextSurface);
    }
  }
}