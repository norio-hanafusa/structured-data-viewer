cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "")
project(StructuredDataViewer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch all dependencies
include(cmake/FetchDependencies.cmake)

# ImGui library (compiled from fetched sources)
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/data/data_node.cpp
    src/data/json_parser.cpp
    src/data/yaml_parser.cpp
    src/data/xml_parser.cpp
    src/data/html_parser.cpp
    src/data/format_detect.cpp
    src/views/tree_view.cpp
    src/views/table_view.cpp
    src/views/editor_panel.cpp
    src/dialogs/edit_dialog.cpp
    src/dialogs/add_dialog.cpp
    src/dialogs/delete_dialog.cpp
    src/history/undo_redo.cpp
    src/dnd/drag_drop.cpp
    src/theme/catppuccin.cpp
    src/stats/statistics.cpp
    src/util/file_io.cpp
    src/util/string_util.cpp
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${pfd_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui_lib
    OpenGL::GL
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
    pugixml::pugixml
)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Windows: use console subsystem (main entry point) for all builds
# The app uses standard main(), not WinMain
