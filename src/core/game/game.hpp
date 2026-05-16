#pragma once

#include "core/config/config_manager.hpp"

namespace core {

enum class GameState
{
    Menu,
    Options,
    Playing,
    Paused,
    GameOver,
    Quit
};

class Game
{
public:
    Game(const ConfigManager& config, const KeyBindings& bindings);
    ~Game() = default;

    void Run();

private:
    void UpdateDiscordPresence();
    void RunMenu();
    void RunOptions();
    void RunGame();
    void RunPaused();
    void RunGameOver();

    void InitGame();
    void SpawnPiece();
    bool CheckCollision(int piece, int rotation, int posX, int posY);
    void LockPiece();
    void ClearLines();
    void DrawBoard();
    int Rotate(int px, int py, int r);

    GameState m_currentState;
    GameState m_previousState;
    int m_score;
    int m_level;
    int m_linesClearedTotal;
    
    int m_board[20][10];
    int m_currentPiece;
    int m_currentRotation;
    int m_currentX;
    int m_currentY;
    int m_nextPiece;
    int m_dropTimer;

    // Key bindings
    int m_keyLeft;
    int m_keyRight;
    int m_keySoftDrop;
    int m_keyRotate;
    int m_keyHardDrop;
    int m_keyPause;
    int m_keyQuit;
    int m_optionIndex;
    bool m_isBinding;
    int m_musicVolume;
    
    ConfigManager m_config;
};

} // namespace core