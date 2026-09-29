set(BUILD_SHARED_LIBS OFF)
add_subdirectory(janet janet_build)

set(PHYSFS_INSTALL OFF)
set(PHYSFS_PLATFORM_SDL OFF)
set(PHYSFS_BUILD_STATIC ON)
set(PHYSFS_BUILD_SHARED OFF)
set(PHYSFS_BUILD_TEST OFF)
set(PHYSFS_BUILD_DOCS OFF)
add_subdirectory(physfs physfs_build)

set(BUILD_EXAMPLES OFF)
add_subdirectory(raylib raylib_build)

add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/raygui/src")

add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library(rlImGui STATIC
  rlImGui/rlImgui.cpp)
target_include_directories(rlImGui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/rlImGui")
target_link_libraries(rlImGui PUBLIC
  imgui
  raylib)

add_subdirectory(spdlog spdlog_build)

add_subdirectory(raylib-physfs)

add_library(fredbuf STATIC
  fredbuf/fredbuf.cpp)
target_include_directories(fredbuf PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/fredbuf")
target_compile_features(fredbuf PUBLIC cxx_std_20)

set(BUILD_QT OFF)
set(BUILD_IMGUI OFF)
set(BUILD_DEMOS OFF)
set(BUILD_TESTS OFF)
set(ZEP_FEATURE_CPP_FILE_SYSTEM OFF)
# Zep declares cmake_minimum_required(VERSION 3.2), which CMake 4.x refuses
# outright. This lets it configure without patching the submodule. 3.13 rather
# than 3.5 so CMP0077 is NEW and the set()s above win over Zep's option()s.
set(CMAKE_POLICY_VERSION_MINIMUM 3.13)
add_subdirectory(zep)
unset(CMAKE_POLICY_VERSION_MINIMUM)
# Zep only exports its include dir via INSTALL_INTERFACE, so consumers linking
# Zep::Zep in-tree get no include path. Add it ourselves.
target_include_directories(Zep PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zep/include>")
