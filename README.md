# Rummikub

A C++ and Qt6 based implementation of the classic Rummikub tile game.

## Features
- **GUI Interface:** Modern interface built using Qt6.
- **AI Player:** Play against an AI opponent capable of calculating complex board state changes.
- **Interactive Board:** Drag and drop tiles visually to create your runs and groups.
- **Rules Enforcement:** Implements core Rummikub rules including the 30-point initial move requirement.

## Prerequisites
- C++17 compliant compiler
- CMake 3.15 or higher
- Qt 6 (Widgets module)

## Building the Project

1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd Rummikub
   ```

2. Create a build directory and run CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Compile the project:
   ```bash
   make
   ```

## Running the Game

After building, execute the compiled binary from the `build` directory:
```bash
./Rummikub
```

## How to Play
1. Enter the number of players.
2. The AI will draw its starting tiles (via prompt if testing, or automatically).
3. Human players draw their 14 initial tiles.
4. On your turn, you can:
   - Drag tiles from your hand to the board to form **Runs** (same color, sequential numbers) or **Groups** (same number, different colors).
   - Rearrange existing tiles on the board.
   - Click "Draw Tile" if you cannot or do not want to place any tiles.
   - Click "Done" to end your turn and validate your moves.
   - Click "Reset" to undo your current moves and start your turn over.

## Note on First Move
Your very first move must consist of placing sets that total **at least 30 points** from your own hand without manipulating existing tiles on the board.
