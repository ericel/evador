#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <SDL.h>
#include <mutex>

class Obstacle {
public:
    // Parameterized constructor
    Obstacle(int x, int y, SDL_Texture* texture, int screenWidth, int screenHeight);
    
    // Destructor
    ~Obstacle();

    // Copy constructor
    Obstacle(const Obstacle& other);

    // Copy assignment operator
    Obstacle& operator=(const Obstacle& other);

    // Move constructor
    Obstacle(Obstacle&& other) noexcept;

    // Move assignment operator
    Obstacle& operator=(Obstacle&& other) noexcept;

    // Render the obstacle on the specified renderer
    void render(SDL_Renderer* renderer);

    // Set the visibility of the obstacle
    void setVisible(bool visibility);

    // Set the position of the obstacle
    void setPosition(int x, int y);

    // Check if the obstacle is visible
    bool isVisible() const;

    // Check if the obstacle is approaching the given Y position (used for collision detection)
    bool isApproaching(int carY) const;

    // Member variables
    int positionx;
    int positiony;
    int screenWidth, screenHeight;

private:
    SDL_Texture* texture;
    SDL_Rect destRect;
    bool visible;
    int appearanceThreshold;
    mutable std::mutex obstacleMutex;  // mutable to allow const member functions to lock the mutex
};

#endif // OBSTACLE_H
