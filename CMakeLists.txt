cmake_minimum_required(VERSION 4.3)
project(click VERSION 0.1 LANGUAGES C CXX)

if (APPLE)
  enable_language(OBJC)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
  PRIVATE
    src/game.cc
    src/implementations.c
    src/main.cc
    src/zep_example.cc
)
target_include_directories(${PROJECT_NAME}
  PRIVATE
    include)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    raylib
    raygui
    PhysFS::PhysFS-static
    rlImGui
    imgui
    spdlog
    janet
    raylib_physfs
    fredbuf
    Zep::Zep
    utf8cpp::utf8cpp
)

if (APPLE)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      "-framework IOKit"
      "-framework Cocoa"
      "-framework OpenGL"
  )
endif()
