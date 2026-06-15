# RayStart

Raylib C++ Starter Project

This is a project template for building with Raylib in C++. It supports linking to an existing build of Raylib on your local machine, or falls back to fetching Raylib and building from source. All three major systems are supported, Windows, MacOS, Linux, and it targets Desktop, and Web using Emscripten. It also has some initial Raylib boilerplate for a cross-platform game (Desktop and Web, more targets in the future).

## Quick Start (Desktop Debug Build)

1. **clone** this repo

```shell
git clone --depth=1 https://github.com/proc0/raystart.git`
```

2. **rename** folder to project name

#### Linux/Mac

```bash
mv raystart mygame
```

#### Windows

```powershell
ren raystart mygame
```

3. **navigate** into project folder

```shell
cd mygame
```

4. **build** and **run** using build script

#### Linux/Mac

```bash
chmod +x ./build.sh
./build.sh --run
```

#### Windows

```powershell
.\build.ps1 --run
```

Sample Output:

<pre>
-- Generating done (0.0s)
-- Build files have been written to: C:/Users/User/Source/raystart/build
-- Host System: Windows
--
-- Windows Desktop Debug Build
--
-- Using local Raylib: C:/raylib/raylib/src
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: C:/Users/User/Source/raystart/build
[ 33%] Building CXX object CMakeFiles/raystart.dir/src/game.cpp.obj
[ 66%] Building CXX object CMakeFiles/raystart.dir/src/main.cpp.obj
[100%] Linking CXX executable raystart.exe
[100%] Built target raystart
</pre>

## Features

- Multiple ways to use Raylib
  - Use a local build (set path in CMakeLists)
  - Fetch and build Raylib from source
  - Fetch and build a custom Raylib build (WIP/TBD)
- Flexible build configuration
  - Change folder names easily
- Supports saving build info in a config file (Optional)
  - Copies build type info, i.e. Debug or Release, for use in app logic
- Supports mulitple platforms and targets
  - Platforms: Windows, MacOS, Linux
  - Targets: Desktop, Web (with Emscripten)
- Ergonomic build script
  - Abstracts CMake commands
  - Intuitive parameters for build options

## Requirements

- [CMake](https://cmake.org)
- [Emscripten](https://emscripten.org)

Tested with the following compilers and toolchains (WIP):

- Linux / MacOS
  - g++
  - make
- Windows
  - [w64devkit](https://github.com/skeeto/w64devkit)

## Build Command Examples

Substitute `./build.sh` for `.\build.ps1` on Windows.

#### Build Desktop Release

`./build.sh --release`

#### Build Desktop Debug and Run

`./build.sh --run`

#### Build Web Debug and Run

`./build.sh --web --run`

#### Build Web Release

`./build.sh --web --release`

#### Build Desktop Debug with CMake arguments

`./build.sh --verbose`

#### Build Desktop Release, with CMake arguments and run

`./build.sh --release --run --verbose`

## Resources

- [Raylib](https://github.com/raysan5/raylib)
- [Raylib Web build instructions](<https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)>)
- [Emscripten](https://emscripten.org)

## Folder structure

<pre>
my-project
├───assets    # project assets
├───build     # build folder
├───build-web # web build folder
├───dist      # install folder
├───dist-web  # web install folder
├───include   # header files
└───src       # source files
</pre>

## Build Script Usage

<pre>
build{.sh|.bat} [--web] [--release] [--run] [OPTIONS]

--web: build for web using Emscripten
--release: build for release
--run: run executable after build
OPTIONS: additional options passed to CMake build command
</pre>
