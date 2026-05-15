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

const int TETROMINO[7][16] = {
    {0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0}, // I
    {0,0,0,0, 0,1,1,0, 0,1,1,0, 0,0,0,0}, // O
    {0,0,0,0, 0,1,0,0, 1,1,1,0, 0,0,0,0}, // T
    {0,0,0,0, 0,1,1,0, 1,1,0,0, 0,0,0,0}, // S
    {0,0,0,0, 1,1,0,0, 0,1,1,0, 0,0,0,0}, // Z
    {0,0,0,0, 1,0,0,0, 1,1,1,0, 0,0,0,0}, // J
    {0,0,0,0, 0,0,1,0, 1,1,1,0, 0,0,0,0}  // L
};

Game::Game() : m_currentState(GameState::Menu), m_score(0), m_level(1), m_linesClearedTotal(0), m_currentPiece(0), m_currentRotation(0), m_currentX(0), m_currentY(0), m_nextPiece(0), m_dropTimer(0)
{
    std::srand(std::time(nullptr));

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
            std::string details = "Score: " + std::to_string(m_score) + " | Lvl: " + std::to_string(m_level);
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
    mvprintw(startY + 3, startX - 2, "Press 'q' to Quit");
    refresh();

    int ch = getch();
    if (ch == '\n')
    {
        clear();
        InitGame();
        m_currentState = GameState::Playing;
        UpdateDiscordPresence();
    }
    else if (ch == 'q' || ch == 'Q')
    {
        m_currentState = GameState::Quit;
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
                    attron(COLOR_PAIR(m_currentPiece + 1) | A_DIM);
                    mvprintw(startY + drawY, startX + drawX * 2, "::");
                    attroff(COLOR_PAIR(m_currentPiece + 1) | A_DIM);
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
    mvprintw(startY + 14, startX + 24, "Left/Right: Move");
    mvprintw(startY + 15, startX + 24, "Up: Rotate");
    mvprintw(startY + 16, startX + 24, "Down: Soft Drop");
    mvprintw(startY + 17, startX + 24, "Space: Hard Drop");
    mvprintw(startY + 18, startX + 24, "p/ESC: Pause");
    mvprintw(startY + 19, startX + 24, "q: Quit to Menu");
}

void Game::RunGame()
{
    int ch = getch();
    if (ch == 'p' || ch == 'P' || ch == 27) // 27 is the ASCII code for ESC
    {
        m_currentState = GameState::Paused;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == 'q' || ch == 'Q')
    {
        m_currentState = GameState::Menu;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == KEY_LEFT)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX - 1, m_currentY))
            m_currentX--;
    }
    else if (ch == KEY_RIGHT)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX + 1, m_currentY))
            m_currentX++;
    }
    else if (ch == KEY_DOWN)
    {
        if (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY + 1))
        {
            m_currentY++;
        }
    }
    else if (ch == KEY_UP)
    {
        int nextRot = m_currentRotation + 1;
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
    }
    else if (ch == ' ')
    {
        int dropped = 0;
        while (!CheckCollision(m_currentPiece, m_currentRotation, m_currentX, m_currentY + 1)) {
            m_currentY++;
            dropped++;
        }
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
    mvprintw(startY + 2, startX - 4, "Press 'p' or ESC to Resume");
    mvprintw(startY + 3, startX - 3, "Press 'q' to Quit to Menu");
    refresh();

    int ch = getch();
    if (ch == 'p' || ch == 'P' || ch == 27)
    {
        m_currentState = GameState::Playing;
        clear();
        UpdateDiscordPresence();
    }
    else if (ch == 'q' || ch == 'Q')
    {
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
        m_currentState = GameState::Menu;
        clear();
        UpdateDiscordPresence();
    }
}

void Game::Run()
{
    UpdateDiscordPresence();
    clear();

    while (m_currentState != GameState::Quit)
    {
        if (m_currentState == GameState::Menu) RunMenu();
        else if (m_currentState == GameState::Playing) RunGame();
        else if (m_currentState == GameState::Paused) RunPaused();
        else if (m_currentState == GameState::GameOver) RunGameOver();

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}