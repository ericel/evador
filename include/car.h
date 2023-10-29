#ifndef CAR_H
#define CAR_H

#include <SDL.h>
#include <SDL_image.h>
#include <memory>
#include <mutex>
#include <iostream>

class Car {
public:
    // Constructor: Initialize the car with initial position and texture
    Car(int x, int y, const std::string& textureFilePath, SDL_Renderer* renderer);

    // Copy constructor
    Car(const Car& other);

    // Move constructor
    Car(Car&& other) noexcept;

    // Copy assignment operator
    Car& operator=(const Car& other);

    // Move assignment operator
    Car& operator=(Car&& other) noexcept;

    // Destructor: Clean up resources
    ~Car();

    // Move the car based on elapsed time (deltaTime)
    void move(float deltaTime);

    // Render the car on the given renderer
    void render(SDL_Renderer* renderer);

    // Get the X position of the car
    int getX() const;

    // Get the Y position of the car
    int getY() const;

    // Set the X position of the car
    void setX(int newX);

    // Set the Y position of the car
    void setY(int newY);

    // Start the car's movement
    void start();

    // Accelerate the car
    void accelerate();

    // Decelerate the car
    void decelerate();

    // Reset the car's position to the specified coordinates
    void reset(int x, int y);

    // Current speed of the car
    float speed;

    // Car dynamics constants
    static const float ACCELERATION_RATE;
    static const float DECELERATION_RATE;
    static const float MAX_SPEED;
    static const int FINISH_LINE_X;
    float distanceCovered = 0.0f;

    // Move the car to the right
    void moveRight();

    // Move the car to the left
    void moveLeft();

    // Get the width of the car
    int getWidth() {
        return 39;
    };

    // Get the height of the car
    int getHeight() {
        return 65;
    };

private:
    int x, y;
    std::shared_ptr<SDL_Texture> texture;
    mutable std::mutex carMutex;  // Mutex to protect car attributes
    std::shared_ptr<SDL_Texture> loadTexture(const std::string& path, SDL_Renderer* renderer);
    static const int START_LINE_Y; // Adjust this value based on your desired starting position in the Y-axis.

    int moveDistance = 10;  // Default move distance to the right
    int moveDistanceLeft = -10;  // Default move distance to the left
};

#endif // CAR_H
