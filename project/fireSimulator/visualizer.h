#include <utility>

#pragma once

class Visualizer {
    int windowWidth;
    int windowHeight;

    std::vector<sf::RectangleShape> buttons;
    std::vector<sf::Text> buttonLabels; // Labels for buttons
    sf::Font font; // For button texts

    std::vector<std::vector<sf::RectangleShape>> tiles;
    std::vector<std::vector<bool>> permanentlyHighlightedTiles;

    std::pair<int, int> lastHighlightedTileCoords = {-1, -1}; // Stores the last highlighted tile's row and column
    std::unordered_map<int, sf::Color> simulationTileColors; // For custom requested tile colors by simulation

    const int MARGIN_FOR_TILES = 1; // Number of pixels between tiles

    sf::Color tileDefaultColor = sf::Color::White; // Default tile color
    sf::Color tileHighlightColor = sf::Color::Yellow; // Highlighted tile color
    sf::Color buttonDefaultColor = sf::Color::White; // Default button color
    sf::Color buttonHighlightColor = sf::Color::Red; // Highlighted button color

    std::shared_ptr<World> world;

    void initializeButtons();
    void loadFont();

    void initializeTiles();
    void resetPermanentlyHighlightedTiles();
    sf::Color getTileColor(int worldWidthPosition, int worldDepthPosition);

public:
    Visualizer(std::shared_ptr<World> world, int width, int height) : window(sf::VideoMode(width, height), "Simulation", sf::Style::Titlebar | sf::Style::Close), world(std::move(world)), windowWidth(width), windowHeight(height) {
        loadFont();
    }

    // Resets the visualizer to its initial state by clearing highlighted tiles, simulation colors, and reinitializing tiles
    void Reset() {
        lastHighlightedTileCoords = {-1, -1}; // Stores the last highlighted tile's row and column
        simulationTileColors.clear();
        resetPermanentlyHighlightedTiles();
        initializeTiles();

        redrawElements();  // Redraw elements to reflect the changes
    }

    // Sets a new World object for the visualizer, reinitializing buttons and tiles to reflect the new world's properties.
    void setWorld(std::shared_ptr<World> worldToSet) {
        world = std::move(worldToSet);
        initializeButtons();
        initializeTiles(); // Re-initialize tiles with new terrain map / world
    }

    sf::RenderWindow window;
    bool isWindowOpen() const;
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }

    int checkButtonClick(sf::Vector2i mousePos, bool applyFeedback);

    std::pair<int, int> getHoveredTileCoords(sf::Vector2i mousePos);
    void highlightTile(int row, int col);
    void permanentlyHighlightTile(int row, int col);
    void updateTileColors(const std::unordered_map<int, sf::Color>& updatedColors);

    void redrawElements();
};



// Checks if the visualization window is still open, allowing the simulation to know when to terminate.
bool Visualizer::isWindowOpen() const {
    return window.isOpen();
}

// Initializes buttons for user interaction with the simulation, setting their properties and arranging them in the UI.
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

// Checks if a button was clicked based on the mouse position and optionally applies visual feedback. It returns the index of the clicked button.
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

void Visualizer::loadFont() {
    if (!font.loadFromFile("./Trueno-wml2.otf")) {
        std::cerr << "Error loading font" << std::endl;
    }
}



// Initializes the grid of tiles representing the simulation space, setting their size, color, and position.
void Visualizer::initializeTiles() {

    int allBordersSize = (world->TilesOnSide() - 1) * MARGIN_FOR_TILES; // There is one less border than number of tiles per axis
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

// Determines the color of a tile based on its terrain height or custom simulation colors.
sf::Color Visualizer::getTileColor(int worldWidthPosition, int worldDepthPosition) {
    int tileIndex = worldWidthPosition * world->TilesOnSide() + worldDepthPosition;

    // Check if there's a custom color for this tile
    auto it = simulationTileColors.find(tileIndex);
    if (it != simulationTileColors.end()) {
        // If a custom color is found, return it
        return it->second;
    }

    float terrainValue = world->GetTileAt(worldWidthPosition, worldDepthPosition)->GetHeight(); // Use terrain value for color
    sf::Color greenTintedColor = sf::Color(0, terrainValue * 255, 0);
    return greenTintedColor;
}

// Determines which tile, if any, the mouse is currently hovering over and returns its coordinates.s
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

// Highlights a tile temporarily, useful for mouse hover effects. It ensures only the most recently hovered tile is highlighted unless it's marked as permanently highlighted.
void Visualizer::highlightTile(int row, int col) {
    bool needRedraw = false;

    // Highlight the previous tile if necessary and it's not permanently highlighted
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

// Toggles the permanent highlight status of a tile, used to mark specific tiles as starting ones for the simulation.
void Visualizer::permanentlyHighlightTile(int row, int col) {
    if (row != -1 && col != -1) {
        permanentlyHighlightedTiles[row][col] = !permanentlyHighlightedTiles[row][col]; // Toggle the permanent highlight state
        tiles[row][col].setFillColor(permanentlyHighlightedTiles[row][col] ? tileHighlightColor : getTileColor(row, col));
        redrawElements();
    }
}

// Resets all tiles to their non-highlighted state.
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
void Visualizer::updateTileColors(const std::unordered_map<int, sf::Color>& updatedColors) {
    for (const auto& [tileIndex, color] : updatedColors) {
        simulationTileColors[tileIndex] = color;
    }
    initializeTiles();
}


// Redraws all visual elements in the window, including tiles and buttons.
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