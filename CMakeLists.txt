cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)

#                         PROJECT
#═══════════════════════════════════════════════════════════════
set(PROJECT_VERSION "0.2.0")
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} DESCRIPTION ${PROJECT_NAME} LANGUAGES CXX)

#                         SETTINGS
#═══════════════════════════════════════════════════════════════
# Raylib settings - defaults to local lib if exists
option(USE_RAYLIB_FETCH "Fetch and build from source" ON)
# TODO: option(USE_RAYLIB_CUSTOM "Fetch and custom build" OFF)
option(USE_RAYLIB_LOCAL "Use a local static build" OFF)

set(DIR_SOURCE "src" CACHE STRING "Project source files folder name")
set(DIR_INCLUDE "include" CACHE STRING "Project header files folder name")
set(DIR_ASSETS "asset" CACHE STRING "Project assets folder name")
set(DIR_INSTALL "dist" CACHE STRING "Project install folder name")
set(FILE_CONFIG "index" CACHE STRING "Optional config file name in include folder")
# Generate compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#                      LOCAL LIB PATHS
#---------------------------------------------------------------
message(STATUS "Host System: ${CMAKE_HOST_SYSTEM_NAME}")
# LOCAL_EM_INCLUDE - Emscripten Include path REQUIRED for WEB
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(OS Windows)
    set(LOCAL_RAYLIB "C:/raylib/raylib/src")
    set(LOCAL_RAYLIB_WEB "C:/raylib/raylib/src/web")
    set(LOCAL_EM_INCLUDE "C:/emsdk/upstream/emscripten/cache/sysroot/include")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(OS MacOS)
    set(LOCAL_RAYLIB "/usr/local/lib/raylib/src")
    set(LOCAL_RAYLIB_WEB "/usr/local/lib/raylib/src/web")
    set(LOCAL_EM_INCLUDE "/usr/local/lib/emsdk/upstream/emscripten/cache/sysroot/include")
else()
    set(OS Linux)
    set(LOCAL_RAYLIB "/usr/local/lib/raylib/src")
    set(LOCAL_RAYLIB_WEB "/usr/local/lib/raylib/src/web")
    set(LOCAL_EM_INCLUDE "/usr/local/lib/emsdk/upstream/emscripten/cache/sysroot/include")
endif()

#                      BUILD VARIABLES
#═══════════════════════════════════════════════════════════════
# DEFAULTS : Linux Desktop Release Build

# Build type flags
set(IS_DESKTOP OFF)
set(IS_WEB OFF)
set(IS_DEBUG OFF)
set(IS_RELEASE OFF)
# Config file flags
set(DEBUG 0)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(IS_DEBUG ON)
    set(DEBUG 1)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    set(IS_RELEASE ON)
endif()

if(PLATFORM STREQUAL Desktop)
    set(IS_DESKTOP ON)
elseif(PLATFORM STREQUAL Web)
    set(IS_WEB ON)
endif()

set(LIBRARIES ${PROJECT_NAME})

# System Status
message(STATUS "\n-- ${CMAKE_SYSTEM_NAME} ${CMAKE_CXX_COMPILER_ID} ${PLATFORM} ${CMAKE_BUILD_TYPE} Build \n--")

#                      Path validations
#---------------------------------------------------------------
# Use local build of Raylib or fallback to fetch
if(USE_RAYLIB_LOCAL)
    # Raylib path - used for both desktop and web
    if(EXISTS "${LOCAL_RAYLIB}")
        message(STATUS "Using local Raylib: ${LOCAL_RAYLIB}")
        # use local only
        set(USE_RAYLIB_FETCH OFF)
    elseif(USE_RAYLIB_FETCH)
        # fallback to fetching Raylib
        set(USE_RAYLIB_LOCAL OFF)
    else()
        message(FATAL_ERROR "SETTINGS ERROR: Raylib not found!: ${LOCAL_RAYLIB}")
    endif()

    # Local Raylib Web path
    if(USE_RAYLIB_LOCAL AND IS_WEB AND NOT EXISTS "${LOCAL_RAYLIB_WEB}")
        message(FATAL_ERROR "SETTINGS ERROR: Raylib for Web not found!: ${LOCAL_RAYLIB_WEB}")
    endif()
endif()
    
# Emscripten include folder path for both local and remote Raylib
if(IS_WEB AND NOT EXISTS "${LOCAL_EM_INCLUDE}")
    message(FATAL_ERROR "SETTINGS ERROR: Emscripten include path not found!: ${LOCAL_EM_INCLUDE}")
endif()

#                         UTILITIES
#═══════════════════════════════════════════════════════════════

include(FetchContent)
function(Fetch lib url tag)
    FetchContent_Declare(${lib}
        GIT_REPOSITORY ${url}
        GIT_TAG        ${tag}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${lib})
endfunction()

#                       PROJECT SETUP
#═══════════════════════════════════════════════════════════════

add_executable(${PROJECT_NAME})
# source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/${DIR_SOURCE}/*.cpp")
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
# header files
set(PROJECT_INCLUDES "${PROJECT_SOURCE_DIR}/${DIR_INCLUDE}")
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDES})
# compile options
if(IS_RELEASE)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -O2
    )
elseif(IS_DEBUG)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -O0
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE 
    -Wall
    -Wpedantic
    -Wno-missing-braces
    -Wunused-result
)

#                         LIB SETUP
#═══════════════════════════════════════════════════════════════
# Local Static Raylib Desktop Setup
if(USE_RAYLIB_LOCAL AND IS_DESKTOP)
    find_library(PATH_RAYLIB raylib PATHS "${LOCAL_RAYLIB}" REQUIRED NO_CMAKE_FIND_ROOT_PATH)

    add_library(raylib STATIC IMPORTED)
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${PATH_RAYLIB}")

    target_include_directories(${PROJECT_NAME} PRIVATE "${LOCAL_RAYLIB}")

    list(APPEND LIBRARIES raylib)

    if(OS STREQUAL Windows)
        list(APPEND LIBRARIES winmm)
    endif()
endif()

# Local Static Raylib Web Setup
if(USE_RAYLIB_LOCAL AND IS_WEB)
    message(STATUS "Using local Raylib Web: ${LOCAL_RAYLIB_WEB}")

    find_library(PATH_RAYLIB_WEB raylib PATHS "${LOCAL_RAYLIB_WEB}" REQUIRED NO_CMAKE_FIND_ROOT_PATH)

    add_library(raylib STATIC IMPORTED)
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${PATH_RAYLIB_WEB}")

    target_include_directories(${PROJECT_NAME} PRIVATE "${LOCAL_RAYLIB}")

    list(APPEND LIBRARIES raylib)
endif()

# Fetch Raylib
if(USE_RAYLIB_FETCH)
    # don't build the supplied examples
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) 
    set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE) 

    Fetch(raylib "https://github.com/raysan5/raylib.git" head)

    list(APPEND LIBRARIES raylib)
endif()

if(IS_WEB)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_include_directories(${PROJECT_NAME} PRIVATE "${LOCAL_EM_INCLUDE}")

    if(USE_RAYLIB_FETCH)
        if(IS_RELEASE)
            target_compile_options(raylib PRIVATE 
                -O2
            )
        elseif(ISDEBUG)
            target_compile_options(raylib PRIVATE 
                -O0
            )

            target_compile_options(${PROJECT_NAME} PRIVATE 
                -fwasm-exceptions
            )
        endif()

        target_compile_options(raylib PRIVATE 
            -Wall 
            -DPLATFORM_WEB 
            -DGRAPHICS_API_OPENGL_ES3
        )
    endif()

    if(IS_DEBUG)
        set(FILE_WEB_SHELL "shell.html")
    else()
        set(FILE_WEB_SHELL "minshell.html")
    endif()

    if(USE_RAYLIB_LOCAL)
        set(PATH_WEB_SHELL "${LOCAL_RAYLIB}/${FILE_WEB_SHELL}")
    else()
        set(PATH_WEB_SHELL "${CMAKE_CURRENT_SOURCE_DIR}/build-web/_deps/raylib-src/src/${FILE_WEB_SHELL}")
    endif()

    # FULL_ES3 requires Raylib to be compiled for Web with OpenGL ES3 (see README requirements)
    target_link_options(${PROJECT_NAME} PRIVATE 
        -sFULL_ES3 
        -sUSE_GLFW=3 
        -sWASM=1
        # -sASYNCIFY=1
        # 12MB
        # -sINITIAL_HEAP=100663296
        -sEXPORT_ES6=1
        -sEXPORTED_RUNTIME_METHODS=["ccall","cwrap","HEAPF32","requestFullscreen"]
        -sEXPORTED_FUNCTIONS=_main
        --preload-file "../${DIR_ASSETS}"
        --shell-file "${PATH_WEB_SHELL}"
    )

    if(IS_DEBUG)
        target_link_options(${PROJECT_NAME} PRIVATE 
            # wasm-exceptinos not cmpatible with ASYNCIFY
            -fwasm-exceptions
            -sABORT_ON_WASM_EXCEPTIONS
            --emrun
        )
    endif()

    if(IS_RELEASE)
        target_link_options(${PROJECT_NAME} PRIVATE 
            -sMINIFY_HTML
        )
    endif()
endif()

#                         LIB LINK
#═══════════════════════════════════════════════════════════════
target_link_libraries(${LIBRARIES})

#                     BUILD INFO CONFIG
#═══════════════════════════════════════════════════════════════
# Get current datetime
execute_process(COMMAND date "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE OUTPUT_DATE)
string(STRIP "${OUTPUT_DATE}" DATE_BUILD)

# Preprocess config file macros if file exists
set(PATH_CONFIG "${PROJECT_SOURCE_DIR}/${DIR_INCLUDE}/${FILE_CONFIG}")
if(EXISTS "${PATH_CONFIG}.h.in")
    configure_file("${PATH_CONFIG}.h.in" "${PATH_CONFIG}.h")
endif()

#                         INSTALL
#═══════════════════════════════════════════════════════════════
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

# Set install folder
if(IS_WEB)
    set(PATH_BUILD "${PROJECT_SOURCE_DIR}/${DIR_INSTALL}-web")
else()
    set(PATH_BUILD "${PROJECT_SOURCE_DIR}/${DIR_INSTALL}")
endif()

# Install files
if (IS_WEB)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PATH_BUILD}" COMPONENT Runtime)
    install(FILES 
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.js"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.wasm"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.data"
        DESTINATION "${PATH_BUILD}" COMPONENT Runtime)
else()
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PATH_BUILD}" COMPONENT Runtime)
endif()