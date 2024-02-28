#pragma once

class Visualizer {
    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text> buttonLabels; // labels for buttons
    sf::Font font;

    std::vector<sf::RectangleShape> tiles;

    std::vector<bool> permanentlyHighlightedTiles;
    int lastHighlightedTileIndex = -1;

    const int MARGIN_FOR_TILES = 2;

    sf::Color tileDefaultColor = sf::Color::White; // Default tile color
    sf::Color tileHighlightColor = sf::Color::Yellow; // Highlighted tile color
    sf::Color buttonDefaultColor = sf::Color::White; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    std::shared_ptr<World> world;

    void loadFont();
    void initializeButtons(int tileSize);
    void initializeTiles(int windowHeight);
    sf::Color getTileColor(int worldWidthPosition, int worldDepthPosition);

public:
    Visualizer(std::shared_ptr<World> world) : window(sf::VideoMode(800, 600), "Simulation", sf::Style::Titlebar | sf::Style::Close), world(world) {
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
    void permanentlyHighlightTile(int index);
    void resetPermanentlyHighlightedTiles();
};

void Visualizer::initializeButtons(int tileSize) {
    loadFont();

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
        label.setFillColor(sf::Color::Black); // Set label color
        buttonLabels.push_back(label);
    }
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

    permanentlyHighlightedTiles.resize(tiles.size(), false); // Initialize all tiles as not permanently highlighted
}

void Visualizer::loadFont() {
    if (!font.loadFromFile("./Trueno-wml2.otf")) {
        std::cerr << "Error loading font" << std::endl;
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
        drawElements();
    }

    lastHighlightedTileIndex = index; // Update the last highlighted tile index
}

void Visualizer::permanentlyHighlightTile(int index) {
    if (index != -1) {
        permanentlyHighlightedTiles[index] = !permanentlyHighlightedTiles[index]; // Toggle the permanent highlight state
        tiles[index].setFillColor(permanentlyHighlightedTiles[index] ? tileHighlightColor : getTileColor(index / world->TilesOnSide(), index % world->TilesOnSide()));
        drawElements();
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

    // Redraw elements to reflect the changes
    drawElements();
}