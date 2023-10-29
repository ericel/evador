#include "car.h"

// Car dynamics constants
const float Car::ACCELERATION_RATE = 1.0f;
const float Car::DECELERATION_RATE = 1.0f;
const float Car::MAX_SPEED = 120.0f;
const int Car::FINISH_LINE_X = 1000;
const int Car::START_LINE_Y = 20; //350

// Constructor: Initialize the car with initial position and texture
Car::Car(int x, int y, const std::string& textureFilePath, SDL_Renderer* renderer)
    : x(x), y(y), speed(0.0f) {
    texture = loadTexture(textureFilePath, renderer);
     // Check if the texture was loaded successfully
    if (!texture) {
        std::cout << "Failed to load texture from " << textureFilePath.c_str() << "SDL_Error: " << SDL_GetError() << std::endl;
    }
}

// Get the X position of the car
int Car::getX() const {
    std::lock_guard<std::mutex> lock(carMutex);
    return x;
}

// Get the Y position of the car
int Car::getY() const {
    std::lock_guard<std::mutex> lock(carMutex);
    return y;
}

// Set the X position of the car
void Car::setX(int newX) {
    std::lock_guard<std::mutex> lock(carMutex);
    x = newX;
}

// Move the car to the right
void Car::moveRight() {
    std::lock_guard<std::mutex> lock(carMutex);
    x += moveDistance;
}

// Move the car to the left
void Car::moveLeft() {
    std::lock_guard<std::mutex> lock(carMutex);
    x += moveDistanceLeft;
}

// Set the Y position of the car
void Car::setY(int newY) {
    std::lock_guard<std::mutex> lock(carMutex);
    y = newY;
}

// Start the car's movement
void Car::start() {
    // Set an initial speed if you want
    speed = 0.0f; // Or any other initial speed value
    std::cout << "Car started with speed: " << speed << std::endl;
}

// Reset the car's position to the specified coordinates
void Car::reset(int x, int y){
    this->x = x; 
    this->y = y;
}

// Accelerate the car
void Car::accelerate() {
    speed += ACCELERATION_RATE;
    if (speed > MAX_SPEED) speed = MAX_SPEED;
}

// Decelerate the car
void Car::decelerate() {
    speed -= DECELERATION_RATE;
    if (speed < 0) speed = 0;
}

// Move the car based on elapsed time (deltaTime)
void Car::move(float deltaTime) {
    std::lock_guard<std::mutex> lock(carMutex);
    
    // Adjust the y coordinate to make the car move upwards
    y -= static_cast<int>(speed * deltaTime); 
    if (y < START_LINE_Y) y = START_LINE_Y; // Ensure car doesn't move beyond the starting line or upper boundary
}

// Render the car on the given renderer
void Car::render(SDL_Renderer* renderer) {
    std::lock_guard<std::mutex> lock(carMutex);
    if (texture) {
        SDL_Rect carQuad = {x, y, 36, 65};
        SDL_RenderCopy(renderer, texture.get(), nullptr, &carQuad);
    }
}

// Load a texture from a file path
std::shared_ptr<SDL_Texture> Car::loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface) {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }
    return std::shared_ptr<SDL_Texture>(newTexture, SDL_DestroyTexture);
}

// Copy Constructor
Car::Car(const Car& other)
    : x(other.x), y(other.y), texture(other.texture), speed(other.speed) {
    // Nothing extra needed, shared_ptr will automatically increase the reference count.
}

// Move Constructor
Car::Car(Car&& other) noexcept
    : x(other.x), y(other.y), texture(std::move(other.texture)), speed(other.speed) {
    other.x = 0;
    other.y = 0;
    other.speed = 0.0f;
    // other.texture is automatically set to nullptr by std::move
}

// Copy Assignment Operator
Car &Car::operator=(const Car& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        texture = other.texture;  // shared_ptr will automatically handle reference counting
        speed = other.speed;
    }
    return *this;
}

// Move Assignment Operator
Car &Car::operator=(Car&& other) noexcept {
    if (this != &other) {
        x = other.x;
        y = other.y;
        texture = std::move(other.texture);
        speed = other.speed;

        other.x = 0;
        other.y = 0;
        other.speed = 0.0f;
        // other.texture is automatically set to nullptr by std::move
    }
    return *this;
}

// Destructor
Car::~Car() {
    // Destructor is called when the shared_ptr's reference count goes to zero,
    // and it will automatically release the SDL_Texture resource.
}
