# Rubik's Cube AI Solver & 3D Engine

A highly optimized C++ Rubik's Cube engine capable of finding optimal solutions using Iterative Deepening A* (IDA*) guided by pre-computed Pattern Databases. This project features a custom multi-threaded 3D graphical visualizer built with Raylib, complete with interactive cube painting, physical structure validation, and smooth 3D slice animations.

<a id="demo-section"></a>
## 🎥 Demo

*[🔗 Link to this Demo section](https://github.com/ashishsps20/Rubik-s-Cube-Solver#demo-section)*

### Terminal Solver
https://github.com/user-attachments/assets/f9bdfed9-58c4-4945-94e9-088a76f3f433

### GUI Solver
https://github.com/user-attachments/assets/9bd21a89-cf49-4625-b1f2-ddda475e3515

### Following the Solver steps in the video

#### Shuffled Cube [B2 D' D2 L'  F'] -
![Screenshot 2023-11-06 104013](https://github.com/Saket2701/Rubiks_Solver/assets/101319476/1ce3b04a-ac32-47f5-82ff-deba81ebee0f)

#### Solution [F L D' B2] - 
![Screenshot_2023-11-06_111227-removebg](https://github.com/Saket2701/Rubiks_Solver/assets/101319476/74541bc6-19e8-4c94-b87e-758889f7415a)


## ✨ Features

- **Extreme Performance**: The Rubik's cube state is mathematically compressed into a 64-bit integer Bitboard, reducing face rotations to single-cycle bitwise operations.
- **AI Graph Search**: Supports multiple search algorithms to find the optimal path to a solved state:
  - Breadth-First Search (BFS)
  - Depth-First Search (DFS)
  - Iterative Deepening DFS (IDDFS)
  - Iterative Deepening A* (IDA*)
- **Corner Pattern Database**: Utilizes a custom heuristic database generator to map out shortest-path corner permutations. The graph depths are heavily compressed into 4-bit NibbleArrays to minimize RAM utilization.
- **Interactive 3D UI**: Fully interactive 3D visualizer using Raylib and OpenGL.
  - **Manual Play**: Use your keyboard to rotate slices (e.g., `F`, `B`, `U`, `D`, `L`, `R`) with full prime move support using `SHIFT`.
  - **Edit / Painter Mode**: Press `E` to enter edit mode, pick a color (1-6), and use 3D mouse raycasting to paint custom sticker configurations on the cube.
  - **Cube Validator**: A failsafe validator prevents the solver from running on physically impossible states (e.g., a cube with 10 green stickers).
  - **Smooth Animations**: Matrix transformations (`rlRotatef`) are dynamically applied to the cube slices to smoothly animate the solver's moves.
  - **Multi-threading**: The AI solver runs on background worker threads (`std::async`) to guarantee a stutter-free 60 FPS UI.

## 🛠️ Technology Stack

- **C++17**
- **Raylib** (3D Rendering & Input handling)
- **CMake** (Build system)
- **Data Structures**: Graph Theory, Bitboards, Hash Maps
- **Concurrency**: `std::async`, `std::future`, `std::thread`

## 🚀 Building & Running

### Prerequisites
- CMake (3.10 or higher)
- A C++ Compiler (GCC, Clang, or MSVC)
- Git (Raylib will be automatically fetched via CMake)

### Compilation

Clone the repository and build using CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Execution

Run the compiled executable:

```bash
./RUBIKS
```

You will be presented with three options in the console terminal:
1. **View Corner Pattern**: Test the heuristic generation.
2. **Solve a Cube in Console**: Run the solver purely in the terminal.
3. **Launch 3D GUI**: Open the interactive 3D application.

## 🎮 GUI Controls

- **Left Mouse Click + Drag**: Orbit / Rotate the camera around the cube.
- **Enter**: Start the Auto-Solver.
- **F, B, U, D, L, R**: Rotate faces clockwise.
- **Hold SHIFT + (F, B, U, D, L, R)**: Rotate faces counter-clockwise (Prime moves).
- **E**: Toggle Paint / Edit Mode.
  - While in Edit mode, press **1-6** to select a color and **Left Click** on stickers to paint them.

## 📂 Project Structure

- `/GUI` - 3D Raylib visualizer, camera math, and event loops.
- `/Model` - Various Rubik's cube state representations (1D Array, 3D Array, Bitboard) inheriting from the `GenericRubiksCube` interface.
- `/PatternDatabases` - Logic for BFS generation, storage, and retrieval of compressed graph heuristics (NibbleArrays).
- `/Solver` - Implementations of graph search algorithms (BFS, DFS, IDDFS, IDA*).
