cmake_minimum_required(VERSION 3.15)
project(ImGui_Project LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE IntelliSense (clangd, VSCode C/C++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Define Source Files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)

# ==============================================================================
# [How to add new source files]
# Whenever you create a new .cpp file for your classes (e.g., src/Game.cpp),
# you MUST explicitly append its path to the 'SOURCES' list below.
# ==============================================================================
set(SOURCES
    src/main.cpp
    src/Machine.cpp
    src/MachineUI.cpp

    # Add your new source files here. Example:
    # src/GameManager.cpp
    # src/Player.cpp

    # ImGui core and backend files
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 2. Create the executable target
add_executable(imgui_app ${SOURCES})

# 3. Configure Include Directories
target_include_directories(imgui_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# 4. Find and Link External Dependencies
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# 5. Link Libraries
target_link_libraries(imgui_app PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    OpenGL::GL
)

# Windows-specific: Hide the background console window for GUI applications
if(WIN32)
    set_target_properties(imgui_app PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
