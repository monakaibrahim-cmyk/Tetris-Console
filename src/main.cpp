#include <ncurses.h>
#include <clocale>

#ifdef ENABLE_DISCORD_RICH_PRESENCE
#include "core/discord/discord_manager.hpp"
#endif

#include "core/game/game.hpp"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

#ifdef ENABLE_DISCORD_RICH_PRESENCE
    core::DiscordManager::Init();
#endif

    std::setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0); // Hide cursor

    ConfigManager config("config.ini");
    KeyBindings bindings;
    config.Load(bindings);

    Game game(config, bindings);
    game.Run();

    endwin();

#ifdef ENABLE_DISCORD_RICH_PRESENCE
    core::DiscordManager::Shutdown();
#endif

    return 0;
}