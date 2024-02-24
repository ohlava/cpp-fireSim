#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> // Include for sf::sleep

#include <iostream>
#include <vector>

// Base class for simulations
class Simulation {
public:
    virtual void update() = 0; // Update simulation state
    virtual void reset() = 0;  // Reset simulation to initial state
};

// Specific simulation
class FireSpreadSimulation : public Simulation {
public:
    void update() override {
        // Update fire spread logic
    }

    void reset() override {
        // Reset fire spread simulation
    }
};

class Visualizer {
    sf::RenderWindow window;
    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::RectangleShape> tiles;
    Simulation* simulation;
    bool simulationRunning = false;
    int lastHighlightedTileIndex = -1;

    const int NUM_TILES_PER_AXIS = 20;
    const int MARGIN_FOR_TILES = 2;

    sf::Color tileDefaultColor = sf::Color::White; // Default tile color
    sf::Color tileHighlightColor = sf::Color::Yellow; // Highlighted tile color
    sf::Color buttonDefaultColor = sf::Color::Green; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    void initializeButtons(int tileSize);
    void initializeTiles(int windowHeight);
    bool handleEvents();
    void drawElements();

public:
    Visualizer(Simulation* sim) : simulation(sim), window(sf::VideoMode(1000, 500), "Simulation", sf::Style::Titlebar | sf::Style::Close) {
        initializeButtons(window.getSize().y / NUM_TILES_PER_AXIS);
        initializeTiles(window.getSize().y);
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

    void run();
};

void Visualizer::initializeButtons(int tileSize) {
    int margin = 50;
    int xPosition = NUM_TILES_PER_AXIS * tileSize + margin;

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
    int allBordersSize = (NUM_TILES_PER_AXIS - 1) * MARGIN_FOR_TILES ; // there is one less border than number of tiles

    // Calculate tile size based on window height, margin and number of tiles
    int tileSize = (windowHeight - allBordersSize) / NUM_TILES_PER_AXIS;

    // Position tiles in a grid
    for (int i = 0; i < NUM_TILES_PER_AXIS; ++i) {
        for (int j = 0; j < NUM_TILES_PER_AXIS; ++j) {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            tile.setFillColor(tileDefaultColor);

            // Calculate position with margin
            int posX = i * (tileSize + MARGIN_FOR_TILES);
            int posY = j * (tileSize + MARGIN_FOR_TILES);
            tile.setPosition(posX, posY);

            tiles.push_back(tile);
        }
    }
}

bool Visualizer::handleEvents() {

    // TODO make reequest for redrawing more sophisticated - based on also the simulation updates
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
                else {
                    btn.setFillColor(buttonDefaultColor); // Reset to default color if not highlighted
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

void Visualizer::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        if (handleEvents()) {
            drawElements();
            clock.restart();
        }

        if (!window.hasFocus()) {
            sf::sleep(sf::milliseconds(80)); // Sleep to reduce CPU usage
        }

        if (clock.getElapsedTime().asSeconds() > 1) { // If idle for more than a second, reduce the frequency of event polling
            sf::sleep(sf::milliseconds(50));
        }
    }
}



int main() {
    FireSpreadSimulation sim;
    Visualizer vis(&sim);
    vis.run();
    return 0;
}