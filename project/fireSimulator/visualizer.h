#pragma once

class Visualizer {
    int windowWidth;
    int windowHeight;

    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text> buttonLabels; // Labels for buttons
    sf::Font font; // For buttons text

    std::vector<sf::RectangleShape> tiles;

    std::vector<bool> permanentlyHighlightedTiles;
    int lastHighlightedTileIndex = -1;
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
    Visualizer(std::shared_ptr<World> world, int width, int height) : window(sf::VideoMode(width, height), "Simulation", sf::Style::Titlebar | sf::Style::Close), world(world), windowWidth(width), windowHeight(height) {
        loadFont();
    }

    void Reset() {
        lastHighlightedTileIndex = -1;
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

    int getHoveredTileIndex(sf::Vector2i mousePos);
    void highlightTile(int index);
    void permanentlyHighlightTile(int index);
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

    permanentlyHighlightedTiles.resize(tiles.size(), false); // Initialize all tiles as not permanently highlighted
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

    for (const auto& tile : tiles) {
        window.draw(tile);
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

    // Unhighlight the previous tile if necessary and it's not permanently highlighted
    if (lastHighlightedTileIndex != -1 && lastHighlightedTileIndex != index && !permanentlyHighlightedTiles[lastHighlightedTileIndex]) {
        int previousRow = lastHighlightedTileIndex / world->TilesOnSide();
        int previousCol = lastHighlightedTileIndex % world->TilesOnSide();
        tiles[lastHighlightedTileIndex].setFillColor(getTileColor(previousRow, previousCol));
        needRedraw = true;
    }

    // Highlight the new tile
    if (index != -1 && !permanentlyHighlightedTiles[index]) {
        tiles[index].setFillColor(tileHighlightColor);
        needRedraw = true;
    }

    if (needRedraw) {
        redrawElements();
    }

    lastHighlightedTileIndex = index; // Update the last highlighted tile index
}

void Visualizer::permanentlyHighlightTile(int index) {
    if (index != -1) {
        permanentlyHighlightedTiles[index] = !permanentlyHighlightedTiles[index]; // Toggle the permanent highlight state
        tiles[index].setFillColor(permanentlyHighlightedTiles[index] ? tileHighlightColor : getTileColor(index / world->TilesOnSide(), index % world->TilesOnSide()));
        redrawElements();
    }
}

void Visualizer::resetPermanentlyHighlightedTiles() {
    // Iterate through all tiles and reset their highlight status
    for (size_t i = 0; i < permanentlyHighlightedTiles.size(); ++i) {
        if (permanentlyHighlightedTiles[i]) { // Check if the tile is permanently highlighted
            // Reset the highlight status
            permanentlyHighlightedTiles[i] = false;

            // Reset the tile color to its original state based on the world's terrain
            int row = i / world->TilesOnSide();
            int col = i % world->TilesOnSide();
            tiles[i].setFillColor(getTileColor(row, col));
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