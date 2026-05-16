#include "game.hpp"
#include <ncurses.h>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#ifdef ENABLE_DISCORD_RICH_PRESENCE
#include "core/discord/discord_manager.hpp"
#endif

#include "core/audio/audio_manager.hpp"

namespace core {

const int TETROMINO[7][16] = {
    {0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0}, // I
    {0,0,0,0, 0,1,1,0, 0,1,1,0, 0,0,0,0}, // O
    {0,0,0,0, 0,1,0,0, 1,1,1,0, 0,0,0,0}, // T
    {0,0,0,0, 0,1,1,0, 1,1,0,0, 0,0,0,0}, // S
    {0,0,0,0, 1,1,0,0, 0,1,1,0, 0,0,0,0}, // Z
    {0,0,0,0, 1,0,0,0, 1,1,1,0, 0,0,0,0}, // J
    {0,0,0,0, 0,0,1,0, 1,1,1,0, 0,0,0,0}  // L
};

Game::Game(const ConfigManager& config, const KeyBindings& bindings) : m_currentState(GameState::Menu), m_previousState(GameState::Menu), m_score(0), m_level(1), m_linesClearedTotal(0), m_currentPiece(0), m_currentRotation(0), m_currentX(0), m_currentY(0), m_nextPiece(0), m_dropTimer(0),
               m_optionIndex(0), m_isBinding(false), m_musicVolume(bindings.musicVolume), m_config(config)
{
    std::srand(std::time(nullptr));

    m_keyLeft = bindings.keyLeft;
    m_keyRight = bindings.keyRight;
    m_keySoftDrop = bindings.keySoftDrop;
    m_keyRotate = bindings.keyRotate;
    m_keyHardDrop = bindings.keyHardDrop;
    m_keyPause = bindings.keyPause;
    m_keyQuit = bindings.keyQuit;

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);   // I
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // O
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK);// T
        init_pair(4, COLOR_GREEN, COLOR_BLACK);  // S
        init_pair(5, COLOR_RED, COLOR_BLACK);    // Z
        init_pair(6, COLOR_BLUE, COLOR_BLACK);   // J
        init_pair(7, COLOR_WHITE, COLOR_BLACK);  // L
    }
}

void Game::UpdateDiscordPresence()
{
#ifdef ENABLE_DISCORD_RICH_PRESENCE
    switch (m_currentState)
    {
        case GameState::Menu:
            core::DiscordManager::SetActivity("In Menu", "Waiting to play", "large", "Tetris");
            break;
        case GameState::Playing:
        {
            std::string details = "Score: " + std::to_string(m_score) + " | Level: " + std::to_string(m_level);
            core::DiscordManager::SetActivity("Playing Tetris", details.c_str(), "large", "Tetris");
            break;
        }
        case GameState::Paused:
        {
            core::DiscordManager::SetActivity("Paused", "Taking a break", "large", "Tetris");
            break;
        }
        case GameState::GameOver:
        {
            std::string details = "Final Score: " + std::to_string(m_score);
            core::DiscordManager::SetActivity("Game Over", details.c_str(), "large", "Tetris");
            break;
        }
        default:
            break;
    }
#endif
}

void Game::RunMenu()
{
    int startX = COLS / 2 - 10;
    int startY = LINES / 2 - 2;
    mvprintw(startY, startX + 5, "TETRIS");
    mvprintw(startY + 2, startX - 2, "Press 'ENTER' to Start");
    mvprintw(startY + 3, startX - 2, "Press 'o' for Options");
    mvprintw(startY + 4, startX - 2, "Press 'q' to Quit");
    refresh();

    int ch = getch();
    if (ch == '\n')
    {
        core::AudioManager::PlayClickSound();
        core::AudioManager::PlayGameMusic();
        clear();
        InitGame();
        m_currentState = GameState::Playing;
        UpdateDiscordPresence();
    }
    else if (ch == 'o' || ch == 'O')
    {
        core::AudioManager::PlayClickSound();
        m_previousState = m_currentState;
        m_currentState = GameState::Options;
        clear();
    }
    else if (ch == 'q' || ch == 'Q')
    {
        core::AudioManager::PlayClickSound();
        m_currentState = GameState::Quit;
    }
}

void Game::RunOptions()
{
    int startX = COLS / 2 - 15;
    int startY = LINES / 2 - 6;

    mvprintw(startY, startX + 5, "OPTIONS / CONTROLS");

    const char* labels[] = {
        "Music Volume:", "Move Left:", "Move Right:", "Rotate:", "Soft Drop:", "Hard Drop:", "Pause:", "Quit:", "Back"
    };
    int* bindings[] = {
        &m_keyLeft, &m_keyRight, &m_keyRotate, &m_keySoftDrop, &m_keyHardDrop, &m_keyPause, &m_keyQuit
    };

    for (int i = 0; i < 9; ++i) {
        if (i == m_optionIndex) attron(A_REVERSE);

        if (i == 0) {
            mvprintw(startY + 2 + i, startX, "%-15s <%3d%%>", labels[i], m_musicVolume);
        } else if (i < 8) {
            const char* keyNameStr = keyname(*(bindings[i - 1]));
            mvprintw(startY + 2 + i, startX, "%-15s %s", labels[i], (m_isBinding && m_optionIndex == i) ? "<PRESS KEY>" : (keyNameStr ? keyNameStr : "UNKNOWN"));
        } else {
            mvprintw(startY + 2 + i, startX, "%s", labels[i]);
        }

        if (i == m_optionIndex) attroff(A_REVERSE);
    }
    refresh();

    int ch = getch();
    if (ch != ERR) {
        if (m_isBinding) {
            core::AudioManager::PlayClickSound();
            *(bindings[m_optionIndex - 1]) = ch;
            m_isBinding = false;
            
            KeyBindings newBindings = {
                m_keyLeft, m_keyRight, m_keySoftDrop,
                m_keyRotate, m_keyHardDrop, m_keyPause, m_keyQuit, m_musicVolume
            };
            m_config.Save(newBindings);
            
            clear();
        } else {
            if (ch == KEY_UP) {
                core::AudioManager::PlayClickSound();
                m_optionIndex = (m_optionIndex > 0) ? m_optionIndex - 1 : 8;
            } else if (ch == KEY_DOWN) {
                core::AudioManager::PlayClickSound();
                m_optionIndex = (m_optionIndex < 8) ? m_optionIndex + 1 : 0;
            } else if (ch == KEY_LEFT && m_optionIndex == 0) {
                if (m_musicVolume > 0) {
                    m_musicVolume -= 5;
                    core::AudioManager::SetMusicVolume(static_cast<float>(m_musicVolume));
                    KeyBindings newBindings = {
                        m_keyLeft, m_keyRight, m_keySoftDrop,
                        m_keyRotate, m_keyHardDrop, m_keyPause, m_keyQuit, m_musicVolume
                    };
                    m_config.Save(newBindings);
                    core::AudioManager::PlayClickSound();
                }
            } else if (ch == KEY_RIGHT && m_optionIndex == 0) {
                if (m_musicVolume < 100) {
                    m_musicVolume += 5;
                    core::AudioManager::SetMusicVolume(static_cast<float>(m_musicVolume));
                    KeyBindings newBindings = {
                        m_keyLeft, m_keyRight, m_keySoftDrop,
                        m_keyRotate, m_keyHardDrop, m_keyPause, m_keyQuit, m_musicVolume
                    };
                    m_config.Save(newBindings);
                    core::AudioManager::PlayClickSound();
                }
            } else if (ch == '\n') {
                core::AudioManager::PlayClickSound();
                if (m_optionIndex == 8) {
                    m_currentState = m_previousState;
                    m_optionIndex = 0;
                    clear();
                } else if (m_optionIndex > 0) {
                    m_isBinding = true;
                    clear();
                }
            }
        }
    }
}

void Game::InitGame()
{
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 10; ++x)
            m_board[y][x] = 0;
            
    m_score = 0;
    m_level = 1;
    m_linesClearedTotal = 0;
    m_nextPiece = std::rand() % 7;
    SpawnPiece();
}

int Game::Rotate(int px, int py, int r)
{
    switch (r % 4) {
        case 0: return py * 4 + px;        // 0 degrees
        case 1: return 12 + py - (px * 4); // 90 degrees
        case 2: return 15 - (py * 4) - px; // 180 degrees
        case 3: return 3 - py + (px * 4);  // 270 degrees
    }
    return 0;
}

bool Game::CheckCollision(int piece, int rotation, int posX, int posY)
{
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            int pi = Rotate(px, py, rotation);
            if (TETROMINO[piece][pi] != 0) {
                int boardX = posX + px;
                int boardY = posY + py;
                
                // Wall bounds and floor logic
                if (boardX < 0 || boardX >= 10 || boardY >= 20) {
                    return true;
                }
                // Collision with placed pieces
                if (boardY >= 0 && m_board[boardY][boardX] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Game::SpawnPiece()
{
    m_currentPiece = m_nextPiece;
    m_nextPiece = std::rand() % 7;
    m_currentRotation = 0;
    m_currentX = 3;
    m_currentY = 0;
    m_dropTimer = 0;
    
    // If we collide exactly when we spawn, the game is over
    if (CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY)) {
        m_currentState = GameState::GameOver;
        core::AudioManager::PlayGameOverMusic();
        clear();
        UpdateDiscordPresence();
    }
}

void Game::LockPiece()
{
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            int pi = Rotate(px, py, m_currentRotation);
            if (TETROMINO[m_currentPiece][pi] != 0) {
                int boardX = m_currentX + px;
                int boardY = m_currentY + py;
                if (boardY >= 0 && boardY < 20 && boardX >= 0 && boardX < 10) {
                    // Save piece offset by +1 so 0 stays completely blank
                    m_board[boardY][boardX] = m_currentPiece + 1;
                }
            }
        }
    }
    ClearLines();
    SpawnPiece();
}

void Game::ClearLines()
{
    int linesCleared = 0;
    for (int py = 0; py < 20; py++) {
        bool full = true;
        for (int px = 0; px < 10; px++) {
            if (m_board[py][px] == 0) full = false;
        }
        if (full) {
            for (int y = py; y > 0; y--) {
                for (int x = 0; x < 10; x++) {
                    m_board[y][x] = m_board[y-1][x];
                }
            }
            for (int x = 0; x < 10; x++) m_board[0][x] = 0;
            linesCleared++;
        }
    }
    
    if (linesCleared > 0) {
        core::AudioManager::PlayLineClearSound();
        int lineScores[] = {0, 100, 300, 500, 800};
        int pointsEarned = lineScores[linesCleared] * m_level;

        // Check for Perfect Clear
        bool isPerfectClear = true;
        for (int py = 0; py < 20; py++) {
            for (int px = 0; px < 10; px++) {
                if (m_board[py][px] != 0) {
                    isPerfectClear = false;
                    break;
                }
            }
            if (!isPerfectClear) break;
        }

        if (isPerfectClear) {
            pointsEarned *= 10; // Apply a 10x multiplier for a Perfect Clear!
        }

        m_score += pointsEarned;
        m_linesClearedTotal += linesCleared;
        m_level = (m_linesClearedTotal / 10) + 1;
        UpdateDiscordPresence();
    }
}

void Game::DrawBoard()
{
    int startX = COLS / 2 - 10;
    int startY = LINES / 2 - 10;

    // Draw boundaries
    for (int y = 0; y < 20; y++) {
        mvprintw(startY + y, startX - 1, "│");
        mvprintw(startY + y, startX + 20, "│");
    }
    mvprintw(startY + 20, startX - 1, "└────────────────────┘");

    // Draw placed blocks on the board
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (m_board[y][x] != 0) {
                attron(COLOR_PAIR(m_board[y][x]));
                mvprintw(startY + y, startX + x * 2, "[]");
                attroff(COLOR_PAIR(m_board[y][x]));
            } else {
                mvprintw(startY + y, startX + x * 2, "  ");
            }
        }
    }

    // Calculate ghost Y
    int ghostY = m_currentY;
    while (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, ghostY + 1)) {
        ghostY++;
    }

    // Draw ghost piece
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (TETROMINO[m_currentPiece][Rotate(px, py, m_currentRotation)] != 0) {
                int drawX = m_currentX + px;
                int drawY = ghostY + py;
                if (drawY >= 0) {
                    attron(COLOR_PAIR(7) | A_DIM);
                    mvprintw(startY + drawY, startX + drawX * 2, "::");
                    attroff(COLOR_PAIR(7) | A_DIM);
                }
            }
        }
    }

    // Draw currently falling piece
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (TETROMINO[m_currentPiece][Rotate(px, py, m_currentRotation)] != 0) {
                int drawX = m_currentX + px;
                int drawY = m_currentY + py;
                if (drawY >= 0) {
                    attron(COLOR_PAIR(m_currentPiece + 1));
                    mvprintw(startY + drawY, startX + drawX * 2, "[]");
                    attroff(COLOR_PAIR(m_currentPiece + 1));
                }
            }
        }
    }

    // Draw Next Piece
    mvprintw(startY, startX + 24, "Next Piece:");
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (TETROMINO[m_nextPiece][Rotate(px, py, 0)] != 0) {
                attron(COLOR_PAIR(m_nextPiece + 1));
                mvprintw(startY + 2 + py, startX + 24 + px * 2, "[]");
                attroff(COLOR_PAIR(m_nextPiece + 1));
            } else {
                mvprintw(startY + 2 + py, startX + 24 + px * 2, "  ");
            }
        }
    }

    // Draw score & level
    mvprintw(startY + 8, startX + 24, "Score: %-10d", m_score);
    mvprintw(startY + 9, startX + 24, "Level: %-10d", m_level);
    mvprintw(startY + 10, startX + 24, "Lines: %-10d", m_linesClearedTotal);
    
    mvprintw(startY + 13, startX + 24, "Controls:");
    mvprintw(startY + 14, startX + 24, "%s/%s: Move", keyname(m_keyLeft), keyname(m_keyRight));
    mvprintw(startY + 15, startX + 24, "%s: Rotate", keyname(m_keyRotate));
    mvprintw(startY + 16, startX + 24, "%s: Soft Drop", keyname(m_keySoftDrop));
    mvprintw(startY + 17, startX + 24, "%s: Hard Drop", keyname(m_keyHardDrop));
    mvprintw(startY + 18, startX + 24, "%s/ESC: Pause", keyname(m_keyPause));
    mvprintw(startY + 19, startX + 24, "%s: Quit to Menu", keyname(m_keyQuit));
}

void Game::RunGame()
{
    int ch = getch();
    if (ch == m_keyPause || ch == 27) // Keep 27 fallback for ESC
    {
        core::AudioManager::PlayClickSound();
        m_currentState = GameState::Paused;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == m_keyQuit)
    {
        core::AudioManager::PlayClickSound();
        core::AudioManager::PlayMenuMusic();
        m_currentState = GameState::Menu;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == m_keyLeft)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX - 1, m_currentY))
            m_currentX--;
    }
    else if (ch == m_keyRight)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX + 1, m_currentY))
            m_currentX++;
    }
    else if (ch == m_keySoftDrop)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY + 1))
        {
            m_currentY++;
        }
    }
    else if (ch == m_keyRotate)
    {
        int nextRot = m_currentRotation + 1;
        int oldRot = m_currentRotation;
        if (!CheckCollision(m_currentPiece, nextRot, m_currentX, m_currentY)) {
            m_currentRotation = nextRot;
        } else if (!CheckCollision(m_currentPiece, nextRot, m_currentX - 1, m_currentY)) {
            m_currentX--; m_currentRotation = nextRot; // Kick Left 1
        } else if (!CheckCollision(m_currentPiece, nextRot, m_currentX + 1, m_currentY)) {
            m_currentX++; m_currentRotation = nextRot; // Kick Right 1
        } else if (!CheckCollision(m_currentPiece, nextRot, m_currentX - 2, m_currentY)) {
            m_currentX -= 2; m_currentRotation = nextRot; // Kick Left 2 (mostly for the 'I' piece)
        } else if (!CheckCollision(m_currentPiece, nextRot, m_currentX + 2, m_currentY)) {
            m_currentX += 2; m_currentRotation = nextRot; // Kick Right 2
        }
        if (m_currentRotation != oldRot) {
            core::AudioManager::PlayRotationSound();
        }
    }
    else if (ch == m_keyHardDrop)
    {
        int dropped = 0;
        while (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY + 1)) {
            m_currentY++;
            dropped++;
        }
        core::AudioManager::PlayDropSound();
        LockPiece();
    }

    if (m_currentState != GameState::Playing) return;

    // Gravity timer logic
    m_dropTimer++;
    int dropThreshold = std::max(5, 40 - (m_level - 1) * 3); // 40 frames @ 60fps ~666ms
    if (m_dropTimer >= dropThreshold)
    {
        m_dropTimer = 0;
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY + 1)) {
            m_currentY++;
        } else {
            LockPiece();
        }
    }

    if (m_currentState == GameState::Playing) {
        DrawBoard();
        refresh();
    }
}

void Game::RunPaused()
{
    int startX = COLS / 2 - 10;
    int startY = LINES / 2 - 2;
    mvprintw(startY, startX + 5, "PAUSED");
    mvprintw(startY + 2, startX - 4, "Press '%s' or ESC to Resume", keyname(m_keyPause));
    mvprintw(startY + 3, startX - 4, "Press 'o' for Options");
    mvprintw(startY + 4, startX - 4, "Press '%s' to Quit to Menu", keyname(m_keyQuit));
    refresh();

    int ch = getch();
    if (ch == m_keyPause || ch == 27)
    {
        core::AudioManager::PlayClickSound();
        m_currentState = GameState::Playing;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == 'o' || ch == 'O')
    {
        core::AudioManager::PlayClickSound();
        m_previousState = m_currentState;
        m_currentState = GameState::Options;
        clear();
    }
    else if (ch == m_keyQuit)
    {
        core::AudioManager::PlayClickSound();
        core::AudioManager::PlayMenuMusic();
        m_currentState = GameState::Menu;
        clear();
        UpdateDiscordPresence();
    }
}

void Game::RunGameOver()
{
    int startX = COLS / 2 - 10;
    int startY = LINES / 2 - 2;
    mvprintw(startY, startX + 3, "GAME OVER");
    mvprintw(startY + 2, startX + 1, "Final Score: %-10d", m_score);
    mvprintw(startY + 4, startX - 3, "Press 'ENTER' to return to Menu");
    refresh();

    int ch = getch();
    if (ch == '\n')
    {
        core::AudioManager::PlayClickSound();
        core::AudioManager::PlayMenuMusic();
        m_currentState = GameState::Menu;
        clear();
        UpdateDiscordPresence();
    }
}

void Game::Run()
{
    core::AudioManager::Init();
    core::AudioManager::SetMusicVolume(static_cast<float>(m_musicVolume));
    core::AudioManager::PlayMenuMusic();
    UpdateDiscordPresence();
    clear();

    while (m_currentState != GameState::Quit)
    {
        if (m_currentState == GameState::Menu) RunMenu();
        else if (m_currentState == GameState::Options) RunOptions();
        else if (m_currentState == GameState::Playing) RunGame();
        else if (m_currentState == GameState::Paused) RunPaused();
        else if (m_currentState == GameState::GameOver) RunGameOver();

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    core::AudioManager::Shutdown();
}

} // namespace core