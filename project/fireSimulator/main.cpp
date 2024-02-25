#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> // Include for sf::sleep
#include <SFML/Window.hpp>

#include <iostream>
#include <utility>
#include <vector>

#include "worldClasses.h"
#include "worldGenerator.h"


// Base class for simulations
class Simulation {
        protected:
        World& world; // Reference to the shared world

        public:
        Simulation(World& world) : world(world) {}

        virtual void update() = 0; // Update simulation state
        virtual void reset() = 0;  // Reset simulation to its initial state
};

// Specific simulation
class FireSpreadSimulation : public Simulation {
public:
    FireSpreadSimulation(World& world) : Simulation(world) {}

    void update() override {
        // Implementation of the fire spread logic, interacting with the shared world
    }

    void reset() override {
        // Reset the fire spread simulation state, possibly also resetting relevant parts of the world
    }
};




class Visualizer {
    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::RectangleShape> tiles;

    int lastHighlightedTileIndex = -1;

    const int MARGIN_FOR_TILES = 2;

    sf::Color tileDefaultColor = sf::Color::White; // Default tile color
    sf::Color tileHighlightColor = sf::Color::Yellow; // Highlighted tile color
    sf::Color buttonDefaultColor = sf::Color::Green; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    std::shared_ptr<World> world;
    const int tilesPerAxis = 20; // TODO delete / read from world size

    void initializeButtons(int tileSize);
    void initializeTiles(int windowHeight);

public:
    Visualizer(std::shared_ptr<World> world) : window(sf::VideoMode(1000, 500), "Simulation", sf::Style::Titlebar | sf::Style::Close),
                              world(world) {
    }

    void setWorld(std::shared_ptr<World> worldToSet) {
        world = std::move(worldToSet);
        initializeButtons(window.getSize().y / tilesPerAxis);
        initializeTiles(window.getSize().y); // Re-initialize tiles with terrain map
    }

    // Methods to change colors dynamically if needed
    void setTileColors(sf::Color defaultColor, sf::Color highlightColor) {
        tileDefaultColor = defaultColor;
        tileHighlightColor = highlightColor;
    }

    void setButtonColors(sf::Color defaultColor, sf::Color highlightColor) {
        buttonDefaultColor = defaultColor;
        buttonHighlightColor = highlightColor;
    }

    sf::RenderWindow window;

    bool isWindowOpen() const;
    bool handleEvents();
    void drawElements();

    bool pollEvent(sf::Event &event);
    void update();

};

void Visualizer::initializeButtons(int tileSize) {
    int margin = 50;
    int xPosition = tilesPerAxis * tileSize + margin;

    // Initialize buttons with updated positions
    sf::RectangleShape button(sf::Vector2f(100, 50));
    button.setPosition(xPosition, 100); // Positioned to the right of the tiles
    button.setFillColor(buttonDefaultColor);
    buttons.push_back(button);

    sf::RectangleShape button2(sf::Vector2f(100, 50));
    button2.setPosition(xPosition, 200);
    button2.setFillColor(buttonDefaultColor);
    buttons.push_back(button2);
}

void Visualizer::initializeTiles(int windowHeight) {

    int allBordersSize = (tilesPerAxis - 1) * MARGIN_FOR_TILES; // there is one less border than number of tiles
    int tileSize = (windowHeight - allBordersSize) / tilesPerAxis; // Calculate tile size based on window height, margin and number of tiles
    tiles.clear(); // Clear existing tiles if re-initializing

    // Position tiles in a grid
    for (int i = 0; i < tilesPerAxis; ++i) {
        for (int j = 0; j < tilesPerAxis; ++j) {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            float terrainValue = world->GetTileAt(i, j)->GetHeight(); // Use terrain value for color
            sf::Color grayScaleColor = sf::Color(terrainValue * 255, terrainValue * 255, terrainValue * 255);
            tile.setFillColor(grayScaleColor);

            // Calculate position with margin
            int posX = i * (tileSize + MARGIN_FOR_TILES);
            int posY = j * (tileSize + MARGIN_FOR_TILES);
            tile.setPosition(posX, posY);

            tiles.push_back(tile);
        }
    }
}

bool Visualizer::handleEvents() {
    bool needRedraw = false;
    sf::Event event;
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            for (auto& btn : buttons) {
                if (btn.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    btn.setFillColor(buttonHighlightColor);

                    // TODO tell main logic about it - button functionality

                    needRedraw = true;
                }
            }
        }
    }

    // Change color on tile hover, only one can be hovered over at one time
    int newHighlightedTileIndex = -1;
    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            newHighlightedTileIndex = i;
            break;
        }
    }

    if (newHighlightedTileIndex != lastHighlightedTileIndex) {
        if (lastHighlightedTileIndex != -1) {
            tiles[lastHighlightedTileIndex].setFillColor(tileDefaultColor); // Revert color of previously highlighted tile
        }
        if (newHighlightedTileIndex != -1) {
            tiles[newHighlightedTileIndex].setFillColor(tileHighlightColor); // Highlight new tile
        }
        lastHighlightedTileIndex = newHighlightedTileIndex;
        needRedraw = true;
    }

    return needRedraw;
}

void Visualizer::drawElements() {
    window.clear();

    for (const auto& tile : tiles) {
        window.draw(tile);
    }

    for (const auto& btn : buttons) {
        window.draw(btn);
    }

    window.display();
}

bool Visualizer::isWindowOpen() const {
    return window.isOpen();
}

bool Visualizer::pollEvent(sf::Event& event) {
    return window.pollEvent(event);
}

void Visualizer::update() {
    // Combine existing draw and event handling logic
    if (handleEvents()) {
        drawElements();
    }
}




class MainLogic {
private:
    std::shared_ptr<World> world; // World object to hold the simulation state
    Map<float> terrainMap;

    //FireSpreadSimulation simulation; // The simulation logic TODO add to constructor in MainLogic
    Visualizer visualizer; // The visualizer for rendering

    enum class GameState {
        PreStart, Running, Stopped
    } state; // Game state

public:
    MainLogic(int worldWidthAndDepth) : world(nullptr), terrainMap(worldWidthAndDepth, worldWidthAndDepth),
                                        visualizer(std::make_shared<World>(worldWidthAndDepth, worldWidthAndDepth)), state(GameState::PreStart) {

        WorldGenerator worldGenerator(worldWidthAndDepth, worldWidthAndDepth, 0.15f, 3);
        world = worldGenerator.Generate();
        visualizer.setWorld(world);
        visualizer.drawElements();
    }

    void run() {
        // Main loop to handle game state transitions and updates
        sf::Clock clock;
        while (visualizer.isWindowOpen()) {
            if (visualizer.handleEvents()) {
                visualizer.drawElements();
                clock.restart();
            }

            if (!visualizer.window.hasFocus()) {
                sf::sleep(sf::milliseconds(80)); // Sleep to reduce CPU usage
            }

            if (clock.getElapsedTime().asSeconds() > 1) { // If idle for more than a second, reduce the frequency of event polling
                sf::sleep(sf::milliseconds(50));
            }
        }
    }

private:
    void handleInput() {
        // Handle user input to change game state or interact with the world
        // This could involve starting/stopping the simulation, setting tiles on fire, etc.
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
        visualizer.update(); // Update the visualization based on the new simulation state
    }

    // Other methods as needed for game logic
};

int main() {
    MainLogic logic(20); // Create the game logic with a 100x100 world
    logic.run(); // Run the game
    return 0;
}