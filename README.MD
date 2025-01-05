https://www.youtube.com/watch?v=5n6zpBgvbO8


# Audio-Driven NURBS Surface Visualization

This project visualizes audio input as a dynamic 3D grid using OpenGL and RtAudio. The amplitude of the audio signal is used to control the height of the grid points, creating a wave-like effect. The lines are colored green, with brightness proportional to the amplitude.

## Features
- Real-time audio input visualization.
- Interactive 3D grid with rotation and zoom.
- Green lines with brightness based on audio amplitude.

## Requirements
- **macOS** (tested on macOS, but should work on Linux/Windows with minor adjustments).
- **Xcode Command Line Tools** (for `g++` and `make`).
- **Homebrew** (for installing dependencies).

## Installation

### Step 1: Install Dependencies
Run the following commands to install the required dependencies:

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install rtaudio freeglut


Compile
g++ -o nurbs_surface nurbs_surface.cpp -framework OpenGL -framework GLUT -L/usr/local/lib -lrtaudio -std=c++11 -Wl,-rpath,/usr/local/lib

Run
./nurbs_surface                                                                                                                         

----


```bash
#!/bin/bash

# Install script for Audio-Driven NURBS Surface Visualization

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install dependencies
echo "Installing dependencies..."
brew install rtaudio freeglut

# Compile the code
echo "Compiling the code..."
g++ -o nurbs_surface nurbs_surface.cpp -framework OpenGL -framework GLUT -L/usr/local/lib -lrtaudio -std=c++11 -Wl,-rpath,/usr/local/lib

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Run the program with: ./nurbs_surface"
else
    echo "Compilation failed. Please check the error messages above."
fi
