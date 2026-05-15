#pragma once

enum class GameState
{
    Menu,
    Playing,
    Paused,
    GameOver,
    Quit
};

class Game
{
public:
    Game();
    ~Game() = default;

    void Run();

private:
    void UpdateDiscordPresence();
    void RunMenu();
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
};