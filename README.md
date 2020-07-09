# Game Tutorial [GLFW, OpenGL(GLEW)]

## Setup Instructions

### Mac

1. Run `xcode-select --install` to install the XCode developer tools.
2. Install CMake (available on the website).
   * After installing, if the cmake bin directory is not exported into your shells' config (`.bashrc`, `.bash_profile`, `.zshrc`, etc.) file, add this line manually: `export PATH="/Applications/CMake.app/Contents/bin":$PATH`
3. Open the project folder.
4. Create a `build` folder and `cd` into it.
5. Run `cmake ..`. There should be no errors.
6. Run `make all`. There should be no errors.
7. Now you can launch the game by running `./launch-main.sh`!

## Controls

* Use `W`, `A`, `S`, `D` keys to move the character.
* Press `SpaceBar` to shoot and destroy the cubes.
* Press `M` to toggle the ability to move the camera.
  * Use `↑`, `←`, `↓`, `→` keys to move the camera, and the mouse to look around.
* Press `B` to toggle debug render mode.
* Press `L` to toggle disabling/enabling shadows and lighting.
* Press `J` to disable lighting from the player models' eyes.
* Press `H` to disable lighting from the shot models.
