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
    int worldSize = 30;

    std::unique_ptr<FireSpreadSimulation> fireSpreadSimulation;
    std::vector<Tile*> initBurningTiles; // Initially burning tiles for simulation

    Visualizer visualizer; // The visualizer for rendering

    bool isMouseButtonPressed = false;

    sf::Clock updateClock; // Clock to track time since last simulation update
    float updateInterval = 1.0f; // Interval in seconds between simulation updates

    enum class GameState {
        NewWorld, Running, Stopped
    } state; // Game state

public:
    MainLogic() : world(nullptr), visualizer(std::make_shared<World>(worldSize, worldSize)), state(GameState::NewWorld) {
        generateNewWorld();
    }

    void run() {
        // Main loop to handle game state transitions and updates
        sf::Clock clock;
        while (visualizer.isWindowOpen()) {

            handleInputEvents(clock); // Handle all input events

            update(); // Update game state and simulation

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
                isMouseButtonPressed = true;
                auto mousePos = sf::Mouse::getPosition(visualizer.window);
                handleButtonInteraction(mousePos);
                handleTileInteraction(mousePos);
                clock.restart();
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                isMouseButtonPressed = false;
            }
            else if (event.type == sf::Event::MouseMoved && isMouseButtonPressed) {
                // Handle drag event
                handleTileInteraction(sf::Mouse::getPosition(visualizer.window));
            }
            else if (event.type == sf::Event::MouseMoved) {
                auto mousePos = sf::Mouse::getPosition(visualizer.window);
                int hoveredTileIndex = visualizer.getHoveredTileIndex(mousePos);
                visualizer.highlightTile(hoveredTileIndex);
                clock.restart();
            }
        }
    }

    void handleTileInteraction(const sf::Vector2i& mousePos) {
        if (state == GameState::NewWorld) { // Only allow tile interaction in NewWorld state
            int clickedTileIndex = visualizer.getHoveredTileIndex(mousePos);
            if (clickedTileIndex != -1) {
                Tile* clickedTile = world->GetTileAt(clickedTileIndex / worldSize, clickedTileIndex % worldSize);
                if (std::find(initBurningTiles.begin(), initBurningTiles.end(), clickedTile) == initBurningTiles.end()) {
                    initBurningTiles.push_back(clickedTile);
                    visualizer.permanentlyHighlightTile(clickedTileIndex);
                } else {
                    initBurningTiles.erase(std::remove(initBurningTiles.begin(), initBurningTiles.end(), clickedTile), initBurningTiles.end());
                    visualizer.permanentlyHighlightTile(clickedTileIndex);
                }
            }
        }
    }

    void handleButtonInteraction(const sf::Vector2i& mousePos) {
        int buttonIndex = visualizer.checkButtonClick(mousePos, true);
        if (buttonIndex != -1) {
            std::cout << "Button " << buttonIndex << " clicked" << std::endl;
            switch (buttonIndex) {
                case 0:
                    generateNewWorld();
                    break;
                case 1:
                    startSimulation();
                    break;
                case 2:
                    stopSimulation();
                    break;
                case 3:
                    resetSimulation();
                    break;
            }
        }
    }

    void generateNewWorld() {
        WorldGenerator worldGenerator(worldSize, worldSize, 0.15f, 3);
        world = worldGenerator.Generate();
        visualizer.setWorld(world);
        visualizer.drawElements();

        resetSimulation();
    }

    void startSimulation() {
        if (initBurningTiles.empty() && state == GameState::NewWorld) {
            std::cout << "Ignite some tiles first!" << std::endl;
            return;
        }
        // If simulation is already running or paused, this acts as a continue
        if (state == GameState::Stopped || state == GameState::NewWorld) {
            if (state == GameState::NewWorld) {
                // Only initialize the simulation if it's in the NewWorld state
                fireSpreadSimulation = std::make_unique<FireSpreadSimulation>(*world);
                fireSpreadSimulation->Initialize(initBurningTiles);
            }
            state = GameState::Running;
            updateClock.restart(); // Restart the clock when the simulation starts or continues
            std::cout << "Simulation running" << std::endl;
        }

        world->ResetParameters();

    }

    void stopSimulation() {
        // To add
    }

    void resetSimulation() {
        state = GameState::NewWorld;
        initBurningTiles.clear();

        if (fireSpreadSimulation) {
            fireSpreadSimulation->Reset(); // TODO should also reset world parameters
        }

        visualizer.resetPermanentlyHighlightedTiles();
    }

    void update() {
        switch (state) {
            case GameState::NewWorld:
                // Interaction with the world allowed, but no simulation updates
                break;
            case GameState::Running:
                if (fireSpreadSimulation && updateClock.getElapsedTime().asSeconds() > updateInterval) {
                    fireSpreadSimulation->Update();
                    auto changedTiles = fireSpreadSimulation->GetChangedTiles();
                    // Update the visualizer with changedTiles
                    updateClock.restart(); // Restart the clock after an update
                }
                break;
            case GameState::Stopped:
                // Simulation is paused, no updates
                break;
        }
    }
};

int main() {
    MainLogic logic; // Create the game logic with a 100x100 world
    logic.run(); // Run the game
    return 0;
}