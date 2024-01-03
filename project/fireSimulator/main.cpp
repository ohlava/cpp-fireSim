#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> // Include for sf::sleep

#include <iostream>
#include <vector>

// Function prototypes
void initializeButtons(std::vector<sf::RectangleShape>& buttons, int tileSize);
void initializeTiles(std::vector<sf::RectangleShape>& tiles, int windowHeight);
bool handleEvents(sf::RenderWindow& window, std::vector<sf::RectangleShape>& buttons, std::vector<sf::RectangleShape>& tiles);
void drawElements(sf::RenderWindow& window, const std::vector<sf::RectangleShape>& buttons, const std::vector<sf::RectangleShape>& tiles);

const int NUM_TILES_PER_AXIS = 20;
const int MARGIN_FOR_TILES = 2;

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "Fire Spread Simulator", sf::Style::Titlebar | sf::Style::Close);

    std::vector<sf::RectangleShape> buttons;
    initializeButtons(buttons, window.getSize().y / NUM_TILES_PER_AXIS);

    std::vector<sf::RectangleShape> tiles;
    initializeTiles(tiles, window.getSize().y);

    sf::Clock clock; // Clock to measure idle time
    drawElements(window, buttons, tiles);

    while (window.isOpen()) {

        if (handleEvents(window, buttons, tiles)) {
            drawElements(window, buttons, tiles);
            clock.restart();
        }

        if (!window.hasFocus()) {
            sf::sleep(sf::milliseconds(80));         // Sleep to reduce CPU usage
        }

        if (clock.getElapsedTime().asSeconds() > 1) {
            // If idle for more than 5 seconds, reduce the frequency of event polling
            sf::sleep(sf::milliseconds(50));
        }
    }

    return 0;
}

void initializeButtons(std::vector<sf::RectangleShape>& buttons, int tileSize) {
    int margin = 50;
    int xPosition = NUM_TILES_PER_AXIS * tileSize + margin;

    // Initialize buttons with updated positions
    sf::RectangleShape button(sf::Vector2f(100, 50));
    button.setFillColor(sf::Color::Green);
    button.setPosition(xPosition, 100); // Positioned to the right of the tiles
    buttons.push_back(button);

    sf::RectangleShape button2(sf::Vector2f(100, 50));
    button2.setPosition(xPosition, 200);
    buttons.push_back(button2);
}

void initializeTiles(std::vector<sf::RectangleShape>& tiles, int windowHeight) {
    int allBordersSize = (NUM_TILES_PER_AXIS - 1) * MARGIN_FOR_TILES ; // there is one less border than number of tiles

    // Calculate tile size based on window height, margin and number of tiles
    int tileSize = (windowHeight - allBordersSize) / NUM_TILES_PER_AXIS;

    // Position tiles in a grid
    for (int i = 0; i < NUM_TILES_PER_AXIS; ++i) {
        for (int j = 0; j < NUM_TILES_PER_AXIS; ++j) {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            tile.setFillColor(sf::Color::White);

            // Calculate position with margin
            int posX = i * (tileSize + MARGIN_FOR_TILES);
            int posY = j * (tileSize + MARGIN_FOR_TILES);
            tile.setPosition(posX, posY);

            tiles.push_back(tile);
        }
    }
}

// Returns true if redraw of window is needed based on user actions
bool handleEvents(sf::RenderWindow& window, std::vector<sf::RectangleShape>& buttons, std::vector<sf::RectangleShape>& tiles) {

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
                    btn.setFillColor(sf::Color::Red);

                    // TODO Add button functionality here

                    needRedraw = true;
                }
            }
        }
    }

    // TODO remember last tile, only one can be highlighted at any given time, so we dont need to iterate through all tiles
    // Change color on tile hover
    for (auto& tile : tiles) {
        if (tile.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (tile.getFillColor() != sf::Color::Yellow) {
                tile.setFillColor(sf::Color::Yellow);
                needRedraw = true;
            }
        } else {
            if (tile.getFillColor() != sf::Color::White) {
                tile.setFillColor(sf::Color::White); // Revert color
                needRedraw = true;
            }
        }
    }

    return needRedraw;
}

void drawElements(sf::RenderWindow& window, const std::vector<sf::RectangleShape>& buttons, const std::vector<sf::RectangleShape>& tiles) {
    window.clear();

    for (const auto& tile : tiles) {
        window.draw(tile);
    }

    for (const auto& btn : buttons) {
        window.draw(btn);
    }

    window.display();
}