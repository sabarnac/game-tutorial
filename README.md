# Game Tutorial [GLFW, OpenGL(GLEW)]

## Setup Instructions

### Windows

1. Install CMake (available on the website) and Visual Studio (recommend the Community Edition).
2. Open CMake.
   1. For the first input (`Where is the source code:`), provide the path to the project folder.
   2. For the second input (`Where to build the binaries:`), provide a path to any empty folder that is NOT the project folder (you can also create a folder called `out` inside the project folder and provide the path to that).
   3. Click on `Configure`.
   4. CMake should ask you which compiler to use since the project is being set up for the first time. Select the Visual Studio compiler version that matches the one you installed. (Make sure to select 32-bit or 64-bit based on the version of VS installed and your OS).
   5. Click on `Configure` upto a few more times till all the red lines in the variables window disappear.
   6. Click on `Generate` to generate the Visual Studio project.
3. Open the folder which you provided as the second field input to CMake, and open the project file called `GameTutorial.sln` in Visual Studio.
4. In the menu bar, click on the `Build` menu, and then choose `Build`/`Build Solution`.
5. Click on the play icon button in the row below the menu bar to run the game.

### Linux

1. Install the latest GPU drivers (they can be out-of-date depending on your Linux distro).
2. Install these libraries: `cmake make g++ libx11-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxext-dev libxcursor-dev libxinerama-dev libxi-dev`. These may be named different based on the Linux distro. Should be this in Ubuntu and other Debian based distros. You can refer to Arch Wiki if you run Arch Linux.
3. Open the project folder.
4. Create a `build` folder and `cd` into it.
5. Run `cmake ..`. There should be no errors.
6. Run `make all`. There should be no errors.
7. Now you can launch the game by running `./launch-main.sh`!

### Mac

1. Run `xcode-select --install` to install the XCode developer tools.
2. Install CMake (available on the website).
   - After installing, if the cmake bin directory is not exported into your shells' config (`.bashrc`, `.bash_profile`, `.zshrc`, etc.) file, add this line manually: `export PATH="/Applications/CMake.app/Contents/bin":$PATH`
3. Open the project folder.
4. Create a `build` folder and `cd` into it.
5. Run `cmake ..`. There should be no errors.
6. Run `make all`. There should be no errors.
7. Now you can launch the game by running `./launch-main.sh`!

## Controls

- Use `W`, `A`, `S`, `D` keys to move the character.
- Press `SpaceBar` to shoot and destroy the cubes.
- Press `M` to toggle the ability to move the camera.
  - Use `↑`, `←`, `↓`, `→` keys to move the camera, and the mouse to look around.
- Press `B` to toggle debug render mode.
- Press `T` to toggle debug text.
- Press `L` to toggle disabling/enabling shadows and lighting.
- Press `J` to disable lighting and shadows from the player models' eyes.
- Press `H` to disable lighting and shadows from the shot models.
- Press `V` to toggle VSynv (disabled, enabled single-sync, enabled double-sync).
