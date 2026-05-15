cmake_minimum_required(VERSION 3.10)
project(Tetris VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")

option(ENABLE_LOGGING "Enable File and Console Logging." ON)
option(ENABLE_DISCORD_RICH_PRESENCE "Enables Discord Rich Presence on game." ON)

if (ENABLE_DISCORD_RICH_PRESENCE)
    set(DISCORD_SDK_DIR ${CMAKE_SOURCE_DIR}/extern/discord_game_sdk)
    file(GLOB_RECURSE DISCORD_SDK_SOURCES ${DISCORD_SDK_DIR}/cpp/*.cpp)

    add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${DISCORD_SDK_SOURCES})
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${SRC_DIR}
        ${SRC_DIR}/core
        ${DISCORD_SDK_DIR}/cpp
    )
else()
    add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
        ${SRC_DIR}
    )
endif()

find_package(Threads REQUIRED)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads ${CURSES_LIBRARIES})

if (ENABLE_DISCORD_RICH_PRESENCE)
    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${DISCORD_SDK_DIR}/lib/x86_64)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        discord_game_sdk
    )
endif()

if (ENABLE_LOGGING)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENABLE_LOGGING)
endif()

if (ENABLE_DISCORD_RICH_PRESENCE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENABLE_DISCORD_RICH_PRESENCE)
endif()
