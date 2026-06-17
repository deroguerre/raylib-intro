cmake_minimum_required(VERSION 3.16)
project(raylib-intro CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Télécharge raylib automatiquement
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_SHALLOW    TRUE
)

# Désactive les exemples et jeux de raylib (inutile pour nous)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

# Notre exécutable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Sur Windows : évite l'ouverture d'une console noire en plus de la fenêtre
if(WIN32)
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    else()
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()
