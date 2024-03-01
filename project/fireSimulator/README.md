# FireSimulator

This program uses SFML for basic visualization, designed with a structure that supports relatively easy modifications or extensions. This facilitates the simulation of various environmental and other phenomena influenced by different factors.

## Project Overview

FireSimulator is a C++ project aimed at simulating and visualizing different phenomena (but mainly fire spread) within a grid-based world. The logic is seperated as much as I could and what made sence, so that it is not strongly depandant.

## Key Components

- **World Representation**: The simulation environment is represented by a grid of tiles, each tile having basic static properties like height, moisture, and vegetation type.
- **Simulation Mechanics**: Simulations are built on top of the world representation. Each simulation type (e.g., fire spread) is implemented as a separate module that manipulates the world based on specific rules and parameters.
- **Visualization**: Utilizing SFML for visualization, the project offers a simple yet effective method for real-time simulation result viewing. This includes rendering the world grid and highlighting various tile states (e.g., burning, burned), along with basic UI elements for interaction.

## How to Run

- **Prerequisites**: Make sure SFML 2.5 is installed and properly set up in your development environment.
- **Compilation**: Utilize CMake for building the project. A `CMakeLists.txt` file facilitates easy setup.
- **Execution**: Launch the compiled executable to begin the simulation. Interact with it through UI buttons and mouse interactions to alter the simulation state.

## Logic

- **MainLogic**: Oversees the simulation flow, including world initialization, user input processing, and simulation state updates.
- **World Classes**: Define the simulation world's structure, including the grid of tiles and their attributes.
- **Simulation Classes**: Implement logic for specific simulation types. New simulations can be introduced by extending the `Simulation` class and implementing its virtual methods.
- **World Generation**: Facilitates the creation and configuration of the simulation environment - World. This includes setting up the initial conditions such as tile properties like height, moisture, vegetation.. 
  
## Interacting with the Simulation

- **Starting a Simulation**: Initiate a simulation by clicking the "Start" button, after setting initial conditions (e.g., initial burning tiles for a fire simulation).
- **Pausing/Stopping**: The "Stop" button allows pausing the simulation, which can then be resumed or reset as needed.


## Defining a New Simulation Class

- Inherit from the `Simulation` base class and implement the required methods (Initialize, Update, etc.).
- Use the `ParameterContainer` class to add or adjust custom parameters for your new simulation.
- Integrate your new simulation class with `MainLogic`, adding any necessary UI elements or input handling for your simulation.
  

## Roadmap

Future enhancements may include:

- A dynamic weather system influencing fire spread
- Improved fire spread calculation based on additional factors for realism
- UI improvements and more settings options implemented via buttons, sliders, toggles, etc.

## Support

For support, please open an issue or contact me via email at ohlava@gmail.com.

## Contributing

Contributions are highly encouraged. Send me your ideas and thoughts. If found online, please fork this repository and create a pull request with your changes.

## Authors and Acknowledgement

Project initiated by Ondřej Hlava.

## License

None.

## Project Status

Originally a school project, it's currently not under active development. However, I'm interested in adding more features, simplifying, and speeding up current processes in the future.


## Project Status
The project was created as a school project. It is not active developt right now. But I would like to include more features, simplify and speed up current proccesses etc. in the future.