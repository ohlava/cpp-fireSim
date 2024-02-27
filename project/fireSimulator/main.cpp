#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> // Include for sf::sleep
#include <SFML/Window.hpp>

#include <iostream>
#include <vector>

#include "worldClasses.h"
#include "worldGenerator.h"
#include "visualizer.h"
#include "simulation.h"

class MainLogic {
private:
    std::shared_ptr<World> world; // World object to hold the simulation state
    int worldSize = 20;

    // FireSpreadSimulation simulation;
    Visualizer visualizer; // The visualizer for rendering


    enum class GameState {
        PreStart, Running, Stopped
    } state; // Game state

public:
    MainLogic() : world(nullptr), visualizer(std::make_shared<World>(worldSize, worldSize)), state(GameState::PreStart) {
        generateNewWorld();
    }

    void run() {
        // Main loop to handle game state transitions and updates
        sf::Clock clock;
        while (visualizer.isWindowOpen()) {

            handleInputEvents(clock); // Handle all input events

            if (!visualizer.window.hasFocus()) {
                sf::sleep(sf::milliseconds(70)); // Sleep to reduce CPU usage
            }

            if (clock.getElapsedTime().asSeconds() > 1) { // If idle for more than a second, reduce the frequency of event polling
                sf::sleep(sf::milliseconds(50));
            }
        }
    }

private:
    void handleInputEvents(sf::Clock& clock) {
        // Handle user input to change game state or interact with the world
        sf::Event event;
        while (visualizer.window.pollEvent(event)) {

            if (event.type == sf::Event::Closed) {
                visualizer.window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                auto mousePos = sf::Mouse::getPosition(visualizer.window);
                int buttonIndex = visualizer.checkButtonClick(mousePos, true);
                if (buttonIndex != -1) {
                    std::cout << "Button " << buttonIndex << " clicked" << std::endl;
                    if (buttonIndex == 0) {
                        generateNewWorld();
                    }
                    else if (buttonIndex == 1) {
                        startSimulation();
                    }
                    else if (buttonIndex == 2) {
                        stopSimulation();
                    }
                    else if (buttonIndex == 3) {
                        resetSimulation();
                    }
                }
                clock.restart();
            }
            else if (event.type == sf::Event::MouseMoved) {
                auto mousePos = sf::Mouse::getPosition(visualizer.window);
                int hoveredTileIndex = visualizer.getHoveredTileIndex(mousePos);
                visualizer.highlightTile(hoveredTileIndex);
                clock.restart();
            }
        }
    }

    void generateNewWorld() {
        WorldGenerator worldGenerator(worldSize, worldSize, 0.15f, 3);
        world = worldGenerator.Generate();
        visualizer.setWorld(world);
        visualizer.drawElements();
    }

    void startSimulation() {
        // TO add
    }

    void stopSimulation() {
        // To add
    }

    void resetSimulation() {
        // To add
    }

    void update() {
        // Update the simulation and visualizer based on the current game state
        switch (state) {
            case GameState::PreStart:
                // Allow user to modify the world (e.g., set tiles on fire)
                break;
            case GameState::Running:
                //simulation.update(); // Update the simulation state
                // Pass updated world state to visualizer here
                break;
            case GameState::Stopped:
                // Possibly handle post-simulation logic
                break;
        }
    }
};

int main() {
    MainLogic logic; // Create the game logic with a 100x100 world
    logic.run(); // Run the game
    return 0;
}