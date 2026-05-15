cmake_minimum_required(VERSION 3.10)
project(Tetris VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)

set(ASSET_FILES
    background.mp3
    click.mp3
    drop.mp3
    game_over.mp3
    line_clear.mp3
    main_menu.mp3
    rotation.mp3
)

set(GENERATED_HEADERS)

# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets)

foreach(FILE ${ASSET_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)

    set(INPUT_FILE  ${CMAKE_SOURCE_DIR}/assets/${FILE})
    set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/assets/${FILE_WE}_mp3.h)
    set(VARIABLE_NAME "assets_${FILE_WE}_mp3")

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets
        COMMAND xxd -i -n ${VARIABLE_NAME} ${INPUT_FILE} ${OUTPUT_FILE}
        DEPENDS ${INPUT_FILE}
        VERBATIM
    )

    list(APPEND GENERATED_HEADERS ${OUTPUT_FILE})
endforeach()

add_custom_target(generate_assets DEPENDS ${GENERATED_HEADERS})

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
        ${CMAKE_BINARY_DIR}
        ${DISCORD_SDK_DIR}/cpp
    )
else()
    add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
        ${SRC_DIR}
        ${SRC_DIR}/core
        ${CMAKE_BINARY_DIR}
    )
endif()

add_dependencies(${CMAKE_PROJECT_NAME} generate_assets)

find_package(Threads REQUIRED)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
find_package(SFML COMPONENTS Audio System CONFIG REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads ${CURSES_LIBRARIES} SFML::Audio SFML::System)

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
