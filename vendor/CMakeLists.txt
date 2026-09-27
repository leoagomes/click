set(BUILD_SHARED_LIBS OFF)
add_subdirectory(chibi-scheme chibi-scheme_build)

set(PHYSFS_INSTALL OFF)
set(PHYSFS_PLATFORM_SDL OFF)
set(PHYSFS_BUILD_STATIC ON)
set(PHYSFS_BUILD_SHARED OFF)
set(PHYSFS_BUILD_TESTS OFF)
set(PHYSFS_BUILD_DOCS OFF)
add_subdirectory(physfs physfs_build)

set(BUILD_EXAMPLES OFF)
add_subdirectory(raylib raylib_build)

add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/raygui/src")

add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_table.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_demo.cpp)
target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library(rlImGui STATIC
  rlImGui/rlImgui.cpp)
target_include_directories(rlImGui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/rlImGui")
target_link_libraries(rlImGui PUBLIC
  imgui
  raylib)
