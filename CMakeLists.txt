cmake_minimum_required(VERSION 3.20)

# Set the project name
project(Evador)

# Add C++ 17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SDL2
find_package(SDL2 REQUIRED)

# Find SDL2_image using pkg-config (might need to be installed via brew)
find_package(PkgConfig)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
# Find the text sdl_2 library
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
link_directories(${SDL2_IMAGE_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS})


# Include directories
include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${SDL2_TTF_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
)

# Automatically include all source files from the 'src' directory
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Add the executable
add_executable(Evador ${SOURCES})

# Link libraries
target_link_libraries(
  Evador 
  ${SDL2_LIBRARIES} 
  ${SDL2_IMAGE_LIBRARIES}
  ${SDL2_TTF_LIBRARIES}
  # Add other libraries as needed, e.g., JPEG, PNG
  ${PNG_LIBRARIES}
)

# Copy the assets folder to the build directory
file(COPY ${PROJECT_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_BINARY_DIR}/assets/)
