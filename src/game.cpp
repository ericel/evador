#include "game.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>    // for time()
#include <cstdlib>  // for srand() and rand()

// Constructor for the Game class
Game::Game() {
    // Initialize game state to STARTED (or PAUSED, based on your design)
    GameState gameState = GameState::STARTED;

    // Call the initialization method
    initGame();

    // Seed the random number generator with the current time
    srand(static_cast<unsigned int>(time(nullptr))); 

    // Initialize accumulated time to zero
    accumulatedTime = 0.0f;
}

// The game loop function
void Game::run() {
    while (gameState != GameState::QUIT) {
       
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        timeSinceLastBlink += deltaTime; // where deltaTime is the time (in seconds) since the last frame

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            // user input handler
            handleEvents(e);
        }
        
        // Only update and render if the game state is RUNNING
        if (gameState == GameState::RUNNING) {
            update();  // Update game state
        } 
        render();  // Render game state

         // Used for blicking text 
        if (timeSinceLastBlink > BLINK_INTERVAL) {
            isTextVisible = !isTextVisible;
            timeSinceLastBlink = 0.0f;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));   
    }
}

// Start the game function
void Game::startGame() {
    if (gameState == GameState::RUNNING) {
        scalingEnabled = true;
        car1->start();
        car2->start();
    }
}

// Update game state based on user input and time
void Game::update(){
    // Used for scaling our poor image
   // Update the scale factor here. Adjusting the height scale more than the width.
   if (scalingEnabled) {
    scaleFactor += 0.02f;//0.002f; // Increment the scale factor slightly over time for height
    if (scaleFactor > 5.2f) { // 1.2Reset if it grows too large. Adjust the max value as needed.
        scaleFactor = 5.2f; //1.0f
    }
   }

   // Launch threads to update cars
   std::thread car1UpdateThread(&Car::move, car1.get(), deltaTime * 20);
   std::thread car2UpdateThread(&Car::move, car2.get(), deltaTime);

    // Join threads
    car1UpdateThread.join();
    car2UpdateThread.join();

    accumulatedTime += deltaTime;  // deltaTime should be in seconds

    // Define a constant acceleration value
    const float ACCELERATION = 1.0f; // you can adjust this value based on how fast you want the car to accelerate

    if (accumulatedTime > 0.0f && car2->speed < Car::MAX_SPEED) {
        // Increment the car's speed by the acceleration value
        car2->speed += ACCELERATION;

        // Ensure the speed doesn't exceed MAX_SPEED
        if (car2->speed > Car::MAX_SPEED) {
            car2->speed = Car::MAX_SPEED;
        }
    }
    // Update distance covered
    car1->distanceCovered += car1->speed * deltaTime;
    car2->distanceCovered += car2->speed * deltaTime;

    // Update visibility for obstacles related to car1 (index 0 to 2)
    updateObstacleVisibility(car1->getX(), car1->getY(), 0, 2);
    // Update visibility for obstacles related to car2 (index 3 to 5)
    updateObstacleVisibility(car2->getX(), car2->getY(), 3, 5);

    // Check for collisions for car2 (given car2's obstacles are from index 3 to 5)
    for (int i = 3; i <= 5; ++i) {
        auto& obstacle = obstacles[i];
        if (obstacle.isVisible()) {
            AvoidDirection direction = checkImminentCollision(car2->getX(), car2->getY(), car2->getWidth(), car2->getHeight(), obstacle);
            if (direction == AvoidDirection::Left) {
                car2->moveLeft();
            } else if (direction == AvoidDirection::Right) {
                car2->moveRight();
            }
        }
    }

    // Collision detection for car1
    for (const auto& obstacle : obstacles) {
            if (detectCollision(car1->getX(), car1->getY(), car1->getWidth(), car1->getHeight(), obstacle)) {
                gameState = GameState::GAMEOVER;
                break; // Exit the loop once a collision is detected
            }
    }


}

// This function renders the game
void Game::render() {
    SDL_SetRenderDrawColor(renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer.get());

    // Fill the background with a solid color to mask any potential white space
    SDL_SetRenderDrawColor(renderer.get(), 0x00, 0x00, 0x00, 0xFF); 
    SDL_RenderFillRect(renderer.get(), NULL);

    if (backgroundTexture) {
        int newWidth = 1000; // Keep width constant for this approach
        int newHeight = 634 * scaleFactor;
        int offsetX = (1000 - newWidth) / 2;
        int offsetY = (634 - newHeight) / 2;

        SDL_Rect renderQuad = {offsetX, offsetY, newWidth, newHeight};  // Only one quad needed now since we're not moving the image vertically.
        SDL_RenderCopy(renderer.get(), backgroundTexture.get(), nullptr, &renderQuad);
    }
    // For simplicity, rendering on the main thread
    car1->render(renderer.get());
    car2->render(renderer.get());

    // Statistics 
    // For car1
    renderStatistics(200, 64, "You", car1->speed, car1->distanceCovered);

    // For car2 (positioning it next car1's statistics for clarity)
    renderStatistics(580, 64, "Computer", car2->speed, car2->distanceCovered);


   for (const auto& obstacle : obstacles) {
    // Check if obstacle is supposed to be visible
    if (obstacle.isVisible()) {
        // Create an SDL rectangle with the obstacle's position and dimensions
        SDL_Rect obstacleRect = { obstacle.positionx, obstacle.positiony, obstacle.screenWidth, obstacle.screenHeight };

        // Render the obstacle
        if (SDL_RenderCopy(renderer.get(), obstacleTexture, nullptr, &obstacleRect) < 0) {
            // SDL_RenderCopy returns -1 on failure. Check the SDL error for more information.
            std::cerr << "SDL_RenderCopy failed: " << SDL_GetError() << std::endl;
        }
    }

    if (gameState == GameState::GAMEOVER) {
        renderGameOverMessage();
    }
}

    SDL_RenderPresent(renderer.get());
}

// Handle user input events
void Game::handleEvents(SDL_Event& e) {
    if (e.type == SDL_QUIT) {
        gameState = GameState::QUIT;
    } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_r:
                std::cout << "Reset key pressed!" << std::endl;
                if (gameState != GameState::RUNNING) {
                    gameState = GameState::RESET;
                    std::cout << "Game state Reset" << std::endl;
                    // Reset car positions
                    car1->reset(car1_initial_x, car1_initial_y);
                    car2->reset(car2_initial_x, car2_initial_y);

                    // Reset obstacle visibility
                    resetObstaclesVisibility();
                }
                break;
            case SDLK_RETURN:
                std::cout << "Starting game!" << std::endl;
                 if (gameState != GameState::RUNNING) {
                    gameState = GameState::RUNNING;
                    std::cout << "Game state changed to RUNNING" << std::endl;
                    startGame(); // This starts the game
                } else {
                    gameState = GameState::STOPPED;
                    std::cout << "Game state changed to STOPPED" << std::endl;
                }
                break;
            case SDLK_b:
                std::cout << "Stop Game!" << std::endl;
                break;
            case SDLK_w:
                car1->accelerate();  // This increase the car's speed
                break;
            case SDLK_s:
                car1->decelerate();  // This  decrease the car's speed
                break;
            case SDLK_d:
                car1->moveRight(); // This turns the car right
                break;
            case SDLK_a:
                car1->moveLeft(); // This turns the car left
                break;
        }
    }
}


AvoidDirection Game::checkImminentCollision(int carX, int carY, int carWidth, int carHeight, const Obstacle& obstacle) {
    int dx = carX - obstacle.positionx;
    int dy = carY - obstacle.positiony;
    int distance = std::sqrt(dx * dx + dy * dy);

    // Define the road boundaries 
    int leftBoundary = 450;  // The x-coordinate where the road starts on the left
    int rightBoundary = 530;  // The x-coordinate where the road ends on the right

    if (distance < 58) {  // 100 is the "imminent collision" distance; adjust as needed
        if (carX > obstacle.positionx && carX + carWidth < rightBoundary) {
            return AvoidDirection::Right;
        } else if (carX < obstacle.positionx && carX > leftBoundary) {
            return AvoidDirection::Left;
        } else if (carX + carWidth > rightBoundary) {  // If the car is near the right boundary, force it to move left
            return AvoidDirection::Left;
        } else if (carX < leftBoundary) {  // If the car is near the left boundary, force it to move right
            return AvoidDirection::Right;
        }
    }

    return AvoidDirection::None;
}
// This function is used by car1 for collision detection with obstacles
bool Game::detectCollision(int carX, int carY, int carWidth, int carHeight, const Obstacle& obstacle) {
    if (carX < obstacle.positionx + obstacle.screenWidth &&
        carX + carWidth > obstacle.positionx &&
        carY < obstacle.positiony + obstacle.screenHeight &&
        carY + carHeight > obstacle.positiony) {
        return true; // Collision detected
    }
    return false; // No collision
}

// Render game over blinking message
void Game::renderGameOverMessage() {
    if (!isTextVisible) {
        return; // Don't render the text if it's not visible
    }
    // Create a color for the text
    SDL_Color textColor = {255, 0, 0}; // This is red; you can adjust as needed

    // Create a surface from the text
    SDL_Surface* textSurface = TTF_RenderText_Solid(largeFont, "You lost to AI", textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    } else {
        // Convert the surface to a texture
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer.get(), textSurface);
        if (textTexture == nullptr) {
            std::cerr << "Unable to create texture from text! SDL Error: " << SDL_GetError() << std::endl;
        } else {
            // Render the texture
            SDL_Rect renderQuad = { (1000 - textSurface->w) / 2, (636 - textSurface->h) / 2, textSurface->w, textSurface->h }; // You can adjust these values to position the text as desired
            SDL_RenderCopy(renderer.get(), textTexture, nullptr, &renderQuad);
            
            // Clean up
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

// Shows obstacles as cars approach
void Game::updateObstacleVisibility(int carX, int carY, int startIdx, int endIdx) {
    for (int i = startIdx; i <= endIdx; ++i) {
        auto& obstacle = obstacles[i];
        
        // If the obstacle is already visible, skip the checks
        if (obstacle.isVisible()) {
            continue;
        }

        // Calculate distance between car and obstacle
        int dx = carX - obstacle.positionx;
        int dy = carY - obstacle.positiony;
        int distance = std::sqrt(dx * dx + dy * dy);

        // Make obstacle visible if the car is within a certain distance
        if (distance < 200) {  // 200 is the threshold distance; change it as needed
            obstacle.setVisible(true);
        }
    }
}

// Reset visibility of obstacles 
void Game::resetObstaclesVisibility() {
    for (auto& obstacle : obstacles) {
        obstacle.setVisible(false);
    }
}


void Game::renderStatistics(int x, int y, const std::string &carName, float carSpeed, float carDistance) {
    SDL_Color textColor = {255, 255, 255}; // White color

    float roundedCarSpeed = floor(carSpeed * 100) / 100.0f;
    float roundedCarDistance = floor(carDistance * 100) / 100.0f;

    std::ostringstream speedStream;
    speedStream << std::fixed << std::setprecision(2) << roundedCarSpeed;
    std::string speedText = carName + " Speed: " + speedStream.str();

    std::ostringstream distanceStream;
    distanceStream << std::fixed << std::setprecision(2) << roundedCarDistance;
    std::string distanceText = carName + " Distance: " + distanceStream.str();
  


    SDL_Surface* speedSurface = TTF_RenderText_Solid(font, speedText.c_str(), textColor);
    if (!speedSurface) {
        // Handle error, maybe log it or return
        return;
    }
    
    SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(renderer.get(), speedSurface);
    int speedWidth = speedSurface->w;
    int speedHeight = speedSurface->h;

    if (!speedSurface) {
    std::cout << "Failed to create speedSurface: " << TTF_GetError() << std::endl;
    return;
    }



    SDL_FreeSurface(speedSurface);

    SDL_Surface* distanceSurface = TTF_RenderText_Solid(font, distanceText.c_str(), textColor);
    if (!distanceSurface) {
        // Handle error, maybe log it or return
        SDL_DestroyTexture(speedTexture);
        return;
    }
    SDL_Texture* distanceTexture = SDL_CreateTextureFromSurface(renderer.get(), distanceSurface);
    int distanceWidth = distanceSurface->w;
    int distanceHeight = distanceSurface->h;
    SDL_FreeSurface(distanceSurface);
if (!distanceSurface) {
    std::cout << "Failed to create distanceSurface: " << TTF_GetError() << std::endl;
    return;
}
    // Render the speed text
    SDL_Rect speedRenderQuad = {x, y, speedWidth, speedHeight};
    SDL_RenderCopy(renderer.get(), speedTexture, nullptr, &speedRenderQuad);

    SDL_DestroyTexture(speedTexture); 

    // Render the distance text below the speed
    SDL_Rect distanceRenderQuad = {x, y + speedHeight + 10, distanceWidth, distanceHeight};
    SDL_RenderCopy(renderer.get(), distanceTexture, nullptr, &distanceRenderQuad);

    SDL_DestroyTexture(distanceTexture); 
}


void Game::initGame() {
    // Initialize SDL and other dependencies
    initSDL();

    // Initialize cars
    initCars();

    // Initialize obstacles
    initObstacles();
}

void Game::initSDL() {
    // Initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create an SDL window and wrap it in a shared_ptr
    window = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Evador", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 634, SDL_WINDOW_SHOWN),
        SDL_DestroyWindow);

    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create an SDL renderer and wrap it in a shared_ptr
    renderer = std::shared_ptr<SDL_Renderer>(
    SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
    SDL_DestroyRenderer);

    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }

    font = TTF_OpenFont("assets/fonts/open_sans/OpenSans-VariableFont_wdth,wght.ttf", 24); // 24 is the font size
    largeFont =  TTF_OpenFont("assets/fonts/open_sans/OpenSans-VariableFont_wdth,wght.ttf", 34); 

    if (!font || !largeFont) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    scaleFactor = 1.0f; //1.0f // Initialize the scale factor to 1 (original size)
    // Load the background texture from the provided path
    backgroundTexture = std::shared_ptr<SDL_Texture>(loadTexture("/Applications/dev/cplusplus/evador/assets/evador.png"), SDL_DestroyTexture);


}

void Game::initCars() {
    car1 = std::make_shared<Car>(car1_initial_x, car1_initial_y, "/Applications/dev/cplusplus/evador/assets/car_1.png", renderer.get());
    car2 = std::make_shared<Car>(car2_initial_x, car2_initial_y, "/Applications/dev/cplusplus/evador/assets/car_2.png", renderer.get());
}

void Game::initObstacles() {
    SDL_Surface* loadedobstacleTextureSurface = IMG_Load("/Applications/dev/cplusplus/evador/assets/obstacle.png");
    if (loadedobstacleTextureSurface) {
        obstacleTexture = SDL_CreateTextureFromSurface(renderer.get(), loadedobstacleTextureSurface);
        SDL_FreeSurface(loadedobstacleTextureSurface);
    } else {
        std::cerr << "Image Load Failed: " << IMG_GetError() << std::endl;
    }

    const int OBSTACLE_WIDTH = 42; // Set this to the width of your obstacle
    const int OBSTACLE_HEIGHT = 42; // Set this to the height of your obstacle

    obstacles.emplace_back(Obstacle(350, 400, obstacleTexture, OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
    obstacles.emplace_back(Obstacle(440, 250, obstacleTexture,  OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
    obstacles.emplace_back(Obstacle(420, 90, obstacleTexture,  OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
    obstacles.emplace_back(Obstacle(620, 400, obstacleTexture, OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
    obstacles.emplace_back(Obstacle(500, 260, obstacleTexture, OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
    obstacles.emplace_back(Obstacle(540, 90, obstacleTexture, OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
}

// Load a texture from the given path
SDL_Texture* Game::loadTexture(const std::string &path) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface) {
        newTexture = SDL_CreateTextureFromSurface(renderer.get(), loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

// Destructor for the Game class
Game::~Game() {
    SDL_Quit();  // Clean up SDL
    TTF_CloseFont(font);
    TTF_Quit();
    font = nullptr;
}
