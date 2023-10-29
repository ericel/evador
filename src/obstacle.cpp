#include "obstacle.h"
#include <utility>  // for std::move

// Parameterized constructor
Obstacle::Obstacle(int x, int y, SDL_Texture* texture, int screenWidth, int screenHeight) 
    : texture(texture), screenWidth(screenWidth), screenHeight(screenHeight), visible(false), positionx(x), positiony(y) {
    // Initialize the destination rect to default values
    destRect.x = 0;
    destRect.y = 0;
    SDL_QueryTexture(texture, NULL, NULL, &destRect.w, &destRect.h);
}

// Destructor
Obstacle::~Obstacle() {}

// Copy constructor
Obstacle::Obstacle(const Obstacle& other)
    : positionx(other.positionx), positiony(other.positiony),
      screenWidth(other.screenWidth), screenHeight(other.screenHeight),
      texture(other.texture), destRect(other.destRect), visible(other.visible),
      appearanceThreshold(other.appearanceThreshold)
{
    // Not copying the mutex
}

// Copy assignment operator
Obstacle& Obstacle::operator=(const Obstacle& other)
{
    if (this == &other)
        return *this;

    positionx = other.positionx;
    positiony = other.positiony;
    screenWidth = other.screenWidth;
    screenHeight = other.screenHeight;
    texture = other.texture;
    destRect = other.destRect;
    visible = other.visible;
    appearanceThreshold = other.appearanceThreshold;

    return *this;
}

// Move constructor
Obstacle::Obstacle(Obstacle&& other) noexcept
    : positionx(std::move(other.positionx)), positiony(std::move(other.positiony)),
      screenWidth(std::move(other.screenWidth)), screenHeight(std::move(other.screenHeight)),
      texture(std::move(other.texture)), destRect(std::move(other.destRect)),
      visible(std::move(other.visible)), appearanceThreshold(std::move(other.appearanceThreshold))
{
    other.texture = nullptr;  // Null out the source object
}

// Move assignment operator
Obstacle& Obstacle::operator=(Obstacle&& other) noexcept
{
    if (this == &other)
        return *this;

    positionx = std::move(other.positionx);
    positiony = std::move(other.positiony);
    screenWidth = std::move(other.screenWidth);
    screenHeight = std::move(other.screenHeight);
    texture = std::move(other.texture);
    destRect = std::move(other.destRect);
    visible = std::move(other.visible);
    appearanceThreshold = std::move(other.appearanceThreshold);

    other.texture = nullptr;  // Null out the source object

    return *this;
}

// Render the obstacle
void Obstacle::render(SDL_Renderer* renderer) {
    std::lock_guard<std::mutex> lock(obstacleMutex);
    if (visible) {
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }
}

// Set the obstacle's visibility
void Obstacle::setVisible(bool visibility) {
    std::lock_guard<std::mutex> lock(obstacleMutex);
    visible = visibility;
}

// Set the obstacle's position
void Obstacle::setPosition(int x, int y) {
    std::lock_guard<std::mutex> lock(obstacleMutex);
    destRect.x = x;
    destRect.y = y;
}

// Check if the obstacle is visible
bool Obstacle::isVisible() const {
    std::lock_guard<std::mutex> lock(obstacleMutex);
    return visible;
}

// Check if the obstacle is approaching
bool Obstacle::isApproaching(int carY) const {
    std::lock_guard<std::mutex> lock(obstacleMutex);
    return carY <= appearanceThreshold && !visible;
}
