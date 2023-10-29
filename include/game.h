#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "car.h"
#include "obstacle.h"
#include <memory>
#include <thread>

// Enum representing the different states of the game
enum class GameState {
    STARTED,
    STOPPED,
    PAUSED,
    RESET,
    RUNNING,
    QUIT,
    GAMEOVER
};

// Enum representing directions to avoid obstacles
enum class AvoidDirection { None, Left, Right };

class Game {
public:
    // Constructor
    Game();

    // Destructor
    ~Game();

    // Main game loop
    void run();

private:
    // Methods

    // Start the game
    void startGame();

    // Initialize the game
    void initGame();

    // Initialize SDL
    void initSDL();

    // Initialize cars
    void initCars();

    // Initialize obstacles
    void initObstacles();

    // Update obstacle visibility
    void updateObstacleVisibility(int carX, int carY, int startIndex, int endIndex);

    // Check for imminent collision
    AvoidDirection checkImminentCollision(int carX, int carY, int carWidth, int carHeight, const Obstacle& obstacle);

    // Detect collision
    bool detectCollision(int carX, int carY, int carWidth, int carHeight, const Obstacle& obstacle);

    // Render game over message
    void renderGameOverMessage();

    // Reset obstacle visibility
    void resetObstaclesVisibility();

    // Load a texture from the given path
    SDL_Texture* loadTexture(const std::string &path);

    // Render statistics on the screen
    void renderStatistics(int x, int y, const std::string &carName, float carSpeed, float carDistance);

    // Handle user input events
    void handleEvents(SDL_Event& e);

    // Update game state and logic
    void update();

    // Render the game
    void render();

    // Variables
    GameState gameState; // Current game state

    AvoidDirection avoidDirection; // Direction to avoid obstacles

    std::shared_ptr<SDL_Window> window; // SDL window
    std::shared_ptr<SDL_Renderer> renderer; // SDL renderer

    std::shared_ptr<SDL_Texture> backgroundTexture;  // The background texture


    Uint32 currentFrameTime;
    Uint32 lastFrameTime;
    float deltaTime;

    float accumulatedTime; // Accumulated time

    float scaleFactor; // Scale factor for the game

    bool scalingEnabled = false; // Scaling enabled flag

    std::shared_ptr<Car> car1; // Player's car
    std::shared_ptr<Car> car2; // AI's car

    int car1_initial_x = 380;
    int car1_initial_y = 550;
    int car2_initial_x = 580;
    int car2_initial_y = 550;

    TTF_Font* font; // Font for text
    TTF_Font* largeFont; // Larger font for text

    std::vector<Obstacle> obstacles; // Vector to store obstacles
    SDL_Texture* obstacleTexture; // Texture for obstacles

    float timeSinceLastBlink = 0.0f;
    const float BLINK_INTERVAL = 0.5f; // Interval for text blinking
    bool isTextVisible = true; // Flag to control text visibility
};

#endif
