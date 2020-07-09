# Game Tutorial [GLFW, OpenGL(GLEW)]

## Setup Instructions

### Mac

1. Run `xcode-select --install`.
2. Install Clang compiler (can use `brew`).
3. Install CMake (available on the website).
4. Open the project folder.
5. Create a `build` folder and `cd` into it.
6. Run `cmake ..`. There should be no errors.
7. Run `make all`. There should be no errors.
8. Now you can launch the game by running `./launch-main.sh`!

## Controls

* Use `W`, `A`, `S`, `D` keys to move the character.
* Press `SpaceBar` to shoot and destroy the cubes.
* Press `M` to toggle the ability to move the camera.
  * Use `↑`, `←`, `↓`, `→` keys to move the camera, and the mouse to look around.
* Press `B` to toggle debug render mode.
* Press `L` to toggle disabling/enabling shadows and lighting.
* Press `J` to disable lighting from the player models' eyes.
* Press `H` to disable lighting from the shot models.