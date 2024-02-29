#include <utility>

#pragma once

class Visualizer {
    int windowWidth;
    int windowHeight;

    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text> buttonLabels; // Labels for buttons
    sf::Font font; // For buttons text

    std::vector<std::vector<sf::RectangleShape>> tiles;
    std::vector<std::vector<bool>> permanentlyHighlightedTiles;

    std::pair<int, int> lastHighlightedTileCoords = {-1, -1}; // Stores the last highlighted tile's row and column
    std::unordered_map<int, sf::Color> simulationTileColors;

    const int MARGIN_FOR_TILES = 2;

    sf::Color tileDefaultColor = sf::Color::White; // Default tile color
    sf::Color tileHighlightColor = sf::Color::Yellow; // Highlighted tile color
    sf::Color buttonDefaultColor = sf::Color::White; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    std::shared_ptr<World> world;

    void loadFont();
    void initializeButtons();
    void initializeTiles();
    void resetPermanentlyHighlightedTiles();
    sf::Color getTileColor(int worldWidthPosition, int worldDepthPosition);

public:
    Visualizer(std::shared_ptr<World> world, int width, int height) : window(sf::VideoMode(width, height), "Simulation", sf::Style::Titlebar | sf::Style::Close), world(std::move(world)), windowWidth(width), windowHeight(height) {
        loadFont();
    }

    void Reset() {
        lastHighlightedTileCoords = {-1, -1}; // Stores the last highlighted tile's row and column
        simulationTileColors.clear();
        resetPermanentlyHighlightedTiles();
        initializeTiles();

        redrawElements();  // Redraw elements to reflect the changes
    }

    void setWorld(std::shared_ptr<World> worldToSet) {
        world = std::move(worldToSet);
        initializeButtons();
        initializeTiles(); // Re-initialize tiles with terrain map
    }

    sf::RenderWindow window;
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }
    bool isWindowOpen() const;

    void redrawElements();

    int checkButtonClick(sf::Vector2i mousePos, bool applyFeedback);

    std::pair<int, int> getHoveredTileCoords(sf::Vector2i mousePos);
    void highlightTile(int row, int col);
    void permanentlyHighlightTile(int row, int col);
    void updateSimulationTileColors(const std::unordered_map<int, sf::Color>& updatedColors);
};

void Visualizer::initializeButtons() {

    int tileSize = windowHeight / world->TilesOnSide();
    int margin = 50;
    int xPosition = world->TilesOnSide() * tileSize + margin;

    const std::vector<std::string> buttonLabelsText = {"New World", "Start", "Stop", "Reset"};
    buttons.clear(); // Clear existing buttons if re-initializing
    buttonLabels.clear();

    // Initialize buttons with updated positions
    for (size_t i = 0; i < buttonLabelsText.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(140, 50));
        button.setPosition(xPosition, 100 + i * 100); // Adjusted for correct positioning
        button.setFillColor(buttonDefaultColor);
        buttons.push_back(button);

        // Create and initialize button label
        sf::Text label(buttonLabelsText[i], font, 20); // Use the font loaded earlier
        label.setPosition(xPosition + 10, 110 + i * 100); // Adjust label position
        label.setFillColor(sf::Color::Black);
        buttonLabels.push_back(label);
    }
}

void Visualizer::initializeTiles() {

    int allBordersSize = (world->TilesOnSide() - 1) * MARGIN_FOR_TILES; // there is one less border than number of tiles
    int tileSize = (windowHeight - allBordersSize) / world->TilesOnSide(); // Calculate tile size based on window height, margin and number of tiles
    tiles = std::vector<std::vector<sf::RectangleShape>>(world->TilesOnSide(), std::vector<sf::RectangleShape>(world->TilesOnSide()));
    permanentlyHighlightedTiles = std::vector<std::vector<bool>>(world->TilesOnSide(), std::vector<bool>(world->TilesOnSide(), false)); // Initialize all tiles as not permanently highlighted

    // Position tiles in a grid
    for (int row = 0; row < world->TilesOnSide(); ++row) {
        for (int col = 0; col < world->TilesOnSide(); ++col) {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            sf::Color tileColor = getTileColor(row, col);
            tile.setFillColor(tileColor);

            int posX = col * (tileSize + MARGIN_FOR_TILES);
            int posY = row * (tileSize + MARGIN_FOR_TILES);
            tile.setPosition(posX, posY);

            tiles[row][col] = tile;
        }
    }
}

void Visualizer::loadFont() {
    if (!font.loadFromFile("./Trueno-wml2.otf")) {
        std::cerr << "Error loading font" << std::endl;
    }
}

sf::Color Visualizer::getTileColor(int worldWidthPosition, int worldDepthPosition) {
    int tileIndex = worldWidthPosition * world->TilesOnSide() + worldDepthPosition;

    // Check if there's a custom color for this tile
    auto it = simulationTileColors.find(tileIndex);
    if (it != simulationTileColors.end()) {
        // If a custom color is found, return it
        return it->second;
    }

    float terrainValue = world->GetTileAt(worldWidthPosition, worldDepthPosition)->GetHeight(); // Use terrain value for color
    sf::Color grayScaleColor = sf::Color(terrainValue * 255, terrainValue * 255, terrainValue * 255);
    return grayScaleColor;
}

void Visualizer::redrawElements() {
    window.clear();

    for (const auto& row : tiles) {
        for (const auto& tile : row) {
            window.draw(tile);
        }
    }

    for (size_t i = 0; i < buttons.size(); ++i) {
        window.draw(buttons[i]);
        window.draw(buttonLabels[i]);
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
                redrawElements(); // Redraw elements to show the feedback
                sf::sleep(sf::milliseconds(50)); // Short delay for feedback visibility
                buttons[i].setFillColor(buttonDefaultColor); // Revert the button color
                redrawElements(); // Redraw elements to show the feedback
            }
            return static_cast<int>(i); // Button was clicked, return its index
        }
    }
    return -1; // No button was clicked
}


std::pair<int, int> Visualizer::getHoveredTileCoords(sf::Vector2i mousePos) {
    for (int row = 0; row < world->TilesOnSide(); ++row) {
        for (int col = 0; col < world->TilesOnSide(); ++col) {
            if (tiles[row][col].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                return {row, col}; // Mouse is over this tile
            }
        }
    }
    return {-1, -1}; // Return an invalid pair if outside the grid / No tile is hovered
}


void Visualizer::highlightTile(int row, int col) {
    bool needRedraw = false;




    // Unhighlight the previous tile if necessary and it's not permanently highlighted
    if (lastHighlightedTileCoords.first != -1 && lastHighlightedTileCoords.second != -1 &&
        !(lastHighlightedTileCoords.first == row && lastHighlightedTileCoords.second == col) && !permanentlyHighlightedTiles[lastHighlightedTileCoords.first][lastHighlightedTileCoords.second]) {

        tiles[lastHighlightedTileCoords.first][lastHighlightedTileCoords.second].setFillColor(getTileColor(lastHighlightedTileCoords.first, lastHighlightedTileCoords.second));
        needRedraw = true;
    }

    // Highlight the new tile
    if (row != -1 && col != -1 && !permanentlyHighlightedTiles[row][col]) {
        tiles[row][col].setFillColor(tileHighlightColor);
        needRedraw = true;
    }

    if (needRedraw) {
        redrawElements();
    }

    lastHighlightedTileCoords = {row, col}; // Update the last highlighted tile index
}

void Visualizer::permanentlyHighlightTile(int row, int col) {
    if (row != -1 && col != -1) {
        permanentlyHighlightedTiles[row][col] = !permanentlyHighlightedTiles[row][col]; // Toggle the permanent highlight state
        tiles[row][col].setFillColor(permanentlyHighlightedTiles[row][col] ? tileHighlightColor : getTileColor(row, col));
        redrawElements();
    }
}

void Visualizer::resetPermanentlyHighlightedTiles() {
    // Iterate through all tiles and reset their highlight status
    for (int row = 0; row < world->TilesOnSide(); ++row) {
        for (int col = 0; col < world->TilesOnSide(); ++col) {
            if (permanentlyHighlightedTiles[row][col]) {
                permanentlyHighlightedTiles[row][col] = false; // Reset the highlight status
                tiles[row][col].setFillColor(getTileColor(row, col)); // Reset the tile color
            }
        }
    }
}

// Method to update tile colors based on simulation output
void Visualizer::updateSimulationTileColors(const std::unordered_map<int, sf::Color>& updatedColors) {
    for (const auto& [tileIndex, color] : updatedColors) {
        simulationTileColors[tileIndex] = color;
    }
    initializeTiles();
}