#!/usr/bin/env bash

command -v cmake >/dev/null 2>&1 || { echo >&2 "CMake is not installed or not in the PATH. Please install CMake and add it to your PATH."; exit 1; }

APP_NAME="${PWD##*/}"
PLATFORM=Desktop
BUILD_TYPE=Debug
BUILD_PATH=build
INSTALL_PATH=dist
SHOULD_RUN=false
SHOULD_INSTALL=false
GENERATOR="Unix Makefiles"

for arg in "$@"
do
  if [ "$arg" == "--web" ]; then
    command -v em++ >/dev/null 2>&1 || { echo >&2 "Emscripten is not installed or not in the PATH. Please install Emscripten to build for Web."; exit 1; }
    PLATFORM=Web
    BUILD_PATH=$BUILD_PATH-web
    INSTALL_PATH=$INSTALL_PATH-web
    shift
  elif [ "$arg" == "--release" ]; then
    PLATFORM=Release
    shift
  elif [ "$arg" == "--run" ]; then
    SHOULD_RUN=true
    shift
  elif [ "$arg" == "--install" ]; then
    SHOULD_INSTALL=true
    shift
  fi
done

# Configure
if [ ! -d "$BUILD_PATH" ]; then
  if [ "$PLATFORM" = "Web" ]; then
    emcmake cmake -S . -B $BUILD_PATH -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPLATFORM=$PLATFORM
  else
    cmake -S . -B $BUILD_PATH -G "$GENERATOR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPLATFORM=$PLATFORM
  fi
fi

if [ $? ! -eq 0 ]; then
  echo >&2 "CONFIGURE ERROR"
  exit 1
fi

# Generate
cmake $BUILD_PATH -DPLATFORM=$PLATFORM -DCMAKE_BUILD_TYPE=$BUILD_TYPE

if [ $? ! -eq 0 ]; then
  echo >&2 "GENERATE ERROR"
  exit 1
fi

# Build
cmake --build $BUILD_PATH "$@"

if [ $? ! -eq 0 ]; then
  echo >&2 "BUILD ERROR"
  exit 1
fi

# Install
if [[ "$SHOULD_INSTALL" == "true" ]]; then
  cmake --install $INSTALL_PATH --component Runtime
fi

# Run
if [[ "$SHOULD_RUN" == "true" ]]; then
  if [[ "$PLATFORM" == "Web" ]]; then
    APP_PATH="$BUILD_PATH/$APP_NAME.html"
    SERVER_PATH=$BUILD_PATH
    if [[ "$SHOULD_INSTALL" == "true" ]]; then
      APP_PATH="$INSTALL_PATH/$APP_NAME.html"
      SERVER_PATH=$INSTALL_PATH
    fi

    if [ ! -f "$APP_PATH" ]; then
      echo >&2 "LAUNCH ERROR: $APP_PATH not found."
      exit 1
    fi

    # Run server
    if [[ "$BUILD_TYPE" == "Debug" ]]; then
      emrun "$APP_PATH"
    else
      echo "NOTE: Web release version does not use --emrun, open http://localhost:8000/$APP_NAME.html in browser."
      python -m http.server -d $SERVER_PATH
    fi
  else
    APP_PATH="./$BUILD_PATH/$APP_NAME"
    if [[ "$SHOULD_INSTALL" == "true" ]]; then
      APP_PATH="./$INSTALL_PATH/$APP_NAME"
    fi

    if [ ! -f "$APP_PATH" ]; then
      echo >&2 "LAUNCH ERROR: $APP_PATH not found."
      exit 1
    fi

    eval "$APP_PATH"
  fi
fi