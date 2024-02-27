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
    sf::Color buttonDefaultColor = sf::Color::White; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    std::shared_ptr<World> world;

    void initializeButtons(int tileSize);
    void initializeTiles(int windowHeight);
    sf::Color getTileColor(int worldWidthPosition, int worldDepthPosition);

public:
    Visualizer(std::shared_ptr<World> world) : window(sf::VideoMode(1000, 500), "Simulation", sf::Style::Titlebar | sf::Style::Close), world(world) {
    }

    void setWorld(std::shared_ptr<World> worldToSet) {
        world = std::move(worldToSet);
        initializeButtons(window.getSize().y / world->TilesOnSide());
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
    void drawElements();

    int checkButtonClick(sf::Vector2i mousePos, bool applyFeedback);

    int getHoveredTileIndex(sf::Vector2i mousePos);
    void highlightTile(int index);

    bool pollEvent(sf::Event &event);
    void update();

};

void Visualizer::initializeButtons(int tileSize) {
    int margin = 50;
    int xPosition = world->TilesOnSide() * tileSize + margin;

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

    int allBordersSize = (world->TilesOnSide() - 1) * MARGIN_FOR_TILES; // there is one less border than number of tiles
    int tileSize = (windowHeight - allBordersSize) / world->TilesOnSide(); // Calculate tile size based on window height, margin and number of tiles
    tiles.clear(); // Clear existing tiles if re-initializing

    // Position tiles in a grid
    for (int i = 0; i < world->TilesOnSide(); ++i) {
        for (int j = 0; j < world->TilesOnSide(); ++j) {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            sf::Color tileColor = getTileColor(i, j);
            tile.setFillColor(tileColor);

            // Calculate position with margin
            int posX = i * (tileSize + MARGIN_FOR_TILES);
            int posY = j * (tileSize + MARGIN_FOR_TILES);
            tile.setPosition(posX, posY);

            tiles.push_back(tile);
        }
    }
}

sf::Color Visualizer::getTileColor(int worldWidthPosition, int worldDepthPosition) {
    float terrainValue = world->GetTileAt(worldWidthPosition, worldDepthPosition)->GetHeight(); // Use terrain value for color
    sf::Color grayScaleColor = sf::Color(terrainValue * 255, terrainValue * 255, terrainValue * 255);
    return grayScaleColor;
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

// Check for button click and apply visual feedback directly
int Visualizer::checkButtonClick(sf::Vector2i mousePos, bool applyFeedback = false) {
    for (size_t i = 0; i < buttons.size(); ++i) {
        if (buttons[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (applyFeedback) {
                // Change the button color to indicate it has been clicked
                buttons[i].setFillColor(buttonHighlightColor);
                drawElements(); // Redraw elements to show the feedback
                sf::sleep(sf::milliseconds(50)); // Short delay for feedback visibility
                buttons[i].setFillColor(buttonDefaultColor); // Revert the button color
                drawElements(); // Redraw elements to show the feedback
            }
            return static_cast<int>(i); // Button was clicked, return its index
        }
    }
    return -1; // No button was clicked
}


int Visualizer::getHoveredTileIndex(sf::Vector2i mousePos) {
    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            return static_cast<int>(i); // Mouse is over this tile
        }
    }
    return -1; // No tile is hovered
}


void Visualizer::highlightTile(int index) {
    bool needRedraw = false;

    // Unhighlight the previous tile if necessary
    if (lastHighlightedTileIndex != -1 && lastHighlightedTileIndex != index) {
        int previousRow = lastHighlightedTileIndex / world->TilesOnSide();
        int previousCol = lastHighlightedTileIndex % world->TilesOnSide();
        tiles[lastHighlightedTileIndex].setFillColor(getTileColor(previousRow, previousCol));
        needRedraw = true;
    }

    // Highlight the new tile
    if (index != -1) {
        tiles[index].setFillColor(tileHighlightColor);
        needRedraw = true;
    } else if (lastHighlightedTileIndex != -1) {
        // If there's no new tile to highlight (index == -1), reset the last highlighted tile
        int lastRow = lastHighlightedTileIndex / world->TilesOnSide();
        int lastCol = lastHighlightedTileIndex % world->TilesOnSide();
        tiles[lastHighlightedTileIndex].setFillColor(getTileColor(lastRow, lastCol));
        needRedraw = true;
    }

    if (needRedraw) {
        drawElements();
    }

    lastHighlightedTileIndex = index; // Update the last highlighted tile index
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
                    std::cout << "button " << buttonIndex << " clicked" << std::endl;
                    // Handle button click, for example, start or stop simulation
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
    MainLogic logic(30); // Create the game logic with a 100x100 world
    logic.run(); // Run the game
    return 0;
}