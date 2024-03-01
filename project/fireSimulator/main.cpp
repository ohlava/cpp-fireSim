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
    int worldSize = 30; // Choose a world size

    std::unique_ptr<FireSpreadSimulation> fireSpreadSimulation;
    std::vector<Tile*> initTiles; // Initially burning tiles for simulation
    std::vector<Tile*> prohibitedTiles; // Initially burning tiles for simulation

    Visualizer visualizer; // The visualizer for rendering

    bool isMouseButtonPressed = false;

    sf::Clock updateClock; // Clock to track time since last simulation update
    float updateInterval = 0.8f; // Interval in seconds between simulation updates

    enum class GameState {
        NewWorld, Running, Stopped
    } state; // Game/simulation state

public:
    MainLogic() : world(nullptr), visualizer(std::make_shared<World>(worldSize, worldSize), 800, 600), state(GameState::NewWorld) {
        generateNewWorld();
    }

    // Main loop to handle game state transitions, input events, and updates. It drives the simulation and rendering process, ensuring the game progresses and reacts to user input.
    void run() {
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

    //Processes user inputs to interact with the simulation or UI elements. Making the simulation interactive, allowing users to modify the simulation state or trigger actions.
    void handleInputEvents(sf::Clock& clock) {
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
            else if (event.type == sf::Event::MouseMoved && isMouseButtonPressed) { // Handle drag event
                handleTileInteraction(sf::Mouse::getPosition(visualizer.window));
            }
            else if (event.type == sf::Event::MouseMoved) {
                auto mousePos = sf::Mouse::getPosition(visualizer.window);
                auto [x, y] = visualizer.getHoveredTileCoords(mousePos);
                visualizer.highlightTile(x , y);
                clock.restart();
            }
        }
    }

    // Allows the user to interact with specific tiles in the world, typically to start or stop the fire.
    void handleTileInteraction(const sf::Vector2i& mousePos) {

        if (state == GameState::NewWorld) { // Only allow tile interaction in NewWorld state
            auto [x, y] = visualizer.getHoveredTileCoords(mousePos);

            if (x != -1 && y != -1) { // Valid tile

                Tile* clickedTile = world->GetTileAt(x, y);
                if (std::find(prohibitedTiles.begin(), prohibitedTiles.end(), clickedTile) != prohibitedTiles.end()) {
                    // This tile is prohibited by simulation, ignore the click and return
                    return;
                }
                if (std::find(initTiles.begin(), initTiles.end(), clickedTile) == initTiles.end()) {
                    initTiles.push_back(clickedTile);
                    visualizer.permanentlyHighlightTile(x, y);
                } else {
                    initTiles.erase(std::remove(initTiles.begin(), initTiles.end(), clickedTile), initTiles.end());
                    visualizer.permanentlyHighlightTile(x, y);
                }
            }
        }
    }

    // Handles clicks on UI buttons. It provides a direct interface for controlling the simulation flow.
    void handleButtonInteraction(const sf::Vector2i& mousePos) {
        int buttonIndex = visualizer.checkButtonClick(mousePos, true);
        if (buttonIndex != -1) {
            switch (buttonIndex) {
                case 0:
                    std::cout << "Clicked on NewWorld" << std::endl;
                    generateNewWorld();
                    break;
                case 1:
                    std::cout << "Clicked on start simulation" << std::endl;
                    startSimulation();
                    break;
                case 2:
                    std::cout << "Clicked on stop simulation" << std::endl;
                    stopSimulation();
                    break;
                case 3:
                    std::cout << "Clicked on reset simulation" << std::endl;
                    resetSimulation();
                    break;
            }
        }
    }


    // Updates the simulation state and visual representation based on the elapsed time. It's the core of the simulation logic, advancing the simulation according to its rules.
    void update() {
        switch (state) {
            case GameState::NewWorld:
                // Interaction with the world allowed, but no simulation updates
                break;
            case GameState::Running:
                if (fireSpreadSimulation && updateClock.getElapsedTime().asSeconds() > updateInterval) {
                    fireSpreadSimulation->Update();
                    auto changedTiles = fireSpreadSimulation->GetChangedTileColors();
                    visualizer.updateTileColors(changedTiles);
                    visualizer.redrawElements();
                    updateClock.restart(); // Restart the clock after an update
                }
                break;
            case GameState::Stopped:
                // Simulation is paused, no updates
                break;
        }
    }

    //  It's a preparatory step before the simulation can run, ensuring it has all necessary initial conditions.
    void initializeSimulation() {
        fireSpreadSimulation = std::make_unique<FireSpreadSimulation>(*world);
        fireSpreadSimulation->Initialize(initTiles);
        prohibitedTiles.clear();
        prohibitedTiles = fireSpreadSimulation->GetProhibitedTiles();
    }

    // Creates and prepares a new simulation world and initializes the visualizer with it.
    void generateNewWorld() {
        WorldGenerator worldGenerator(worldSize, worldSize, 0.15f, 3);
        world = worldGenerator.Generate();
        visualizer.setWorld(world);
        visualizer.redrawElements();

        initializeSimulation();
        resetSimulation();
    }

    // Begins new or continues old simulation based on the current game state.
    void startSimulation() {
        if (initTiles.empty() && state == GameState::NewWorld) {
            std::cout << "Ignite some tiles first!" << std::endl;
            return;
        }
        // If simulation is already running or paused, this acts as continue with simulation
        if (state == GameState::Stopped || state == GameState::NewWorld) {
            if (state == GameState::NewWorld) {
                // Only initialize the simulation if it's in the NewWorld state
                initializeSimulation();
                auto changedTiles = fireSpreadSimulation->GetChangedTileColors();
                visualizer.updateTileColors(changedTiles);
                visualizer.redrawElements();
            }
            state = GameState::Running;
            updateClock.restart(); // Restart the clock when the simulation starts or continues
            std::cout << "Simulation running..." << std::endl;
        }
    }

    // Pauses the simulation, allowing it to be resumed later.
    void stopSimulation() {
        state = GameState::Stopped;
    }

    // Resets the simulation to a new state, clearing any ongoing simulation data. It allows for a complete restart of the simulation, clearing all previous interactions and data.
    void resetSimulation() {
        state = GameState::NewWorld;
        initTiles.clear();

        if (fireSpreadSimulation) {
            fireSpreadSimulation->Reset();
        }

        visualizer.Reset();
    }
};



// Entry to the simulator
int main() {
    MainLogic logic;
    logic.run(); // Run the game
    return 0;
}