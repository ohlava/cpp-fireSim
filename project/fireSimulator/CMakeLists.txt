cmake_minimum_required(VERSION 3.26)
project(fireSimulator)

set(CMAKE_CXX_STANDARD 17)

# Set the path to the SFML directory
set(SFML_DIR "/opt/homebrew/Cellar/sfml/2.6.1/lib/cmake/SFML")
set(SOURCE_FILES
    main.cpp)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

# Include the SFML include directory
include_directories(${SFML_INCLUDE_DIRS})

# Link SFML
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-audio)