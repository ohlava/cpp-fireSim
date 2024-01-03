#include <SFML/Graphics.hpp>
#include <vector>

// Function prototypes
void initializeButtons(std::vector<sf::RectangleShape>& buttons, int tileSize);
void initializeTiles(std::vector<sf::RectangleShape>& tiles, int windowHeight);
void handleEvents(sf::RenderWindow& window, std::vector<sf::RectangleShape>& buttons, std::vector<sf::RectangleShape>& tiles);
void drawElements(sf::RenderWindow& window, const std::vector<sf::RectangleShape>& buttons, const std::vector<sf::RectangleShape>& tiles);

const int NUM_TILES_PER_AXIS = 20;
const int MARGIN_FOR_TILES = 2;

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "Fire Spread Simulator", sf::Style::Titlebar | sf::Style::Close);

    std::vector<sf::RectangleShape> buttons;
    initializeButtons(buttons, window.getSize().y / NUM_TILES_PER_AXIS);

    std::vector<sf::RectangleShape> tiles;
    initializeTiles(tiles, window.getSize().y);

    while (window.isOpen()) {
        handleEvents(window, buttons, tiles);
        drawElements(window, buttons, tiles);
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

void handleEvents(sf::RenderWindow& window, std::vector<sf::RectangleShape>& buttons, std::vector<sf::RectangleShape>& tiles) {
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
                }
            }
        }
    }

    // Change color on tile hover
    for (auto& tile : tiles) {
        if (tile.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            tile.setFillColor(sf::Color::Yellow);
        } else {
            tile.setFillColor(sf::Color::White); // Revert color
        }
    }
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