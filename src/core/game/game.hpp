#pragma once

#include "core/config/config_manager.hpp"

namespace core
{

    /**
     * @brief Represents the current state of the game engine.
     */
    enum class GameState
    {
        Menu,
        Options,
        Playing,
        Paused,
        GameOver,
        Quit
    };

    /**
     * @brief The main Game class responsible for handling Tetris logic, rendering, and state management.
     */
    class Game
    {
    public:
        /**
         * @brief Constructs a new Game instance.
         * @param config The configuration manager used for saving and loading settings.
         * @param bindings The initial key bindings and options loaded from the config.
         */
        Game(const ConfigManager& config, const KeyBindings& bindings);
        ~Game() = default;

        /**
         * @brief Starts the main game loop, continuing until the game is closed.
         */
        void Run();

    private:
        /**
         * @brief Updates the Discord Rich Presence activity based on the current GameState.
         */
        void UpdateDiscordPresence();
        
        /**
         * @brief Executes logic and rendering specific to the Main Menu state.
         */
        void RunMenu();
        
        /**
         * @brief Executes logic and rendering specific to the Options state.
         */
        void RunOptions();
        
        /**
         * @brief Executes active gameplay logic, such as gravity drops and user input.
         */
        void RunGame();
        
        /**
         * @brief Executes logic and rendering specific to the Paused state.
         */
        void RunPaused();
        
        /**
         * @brief Executes logic and rendering specific to the Game Over state.
         */
        void RunGameOver();

        /**
         * @brief Initializes a fresh game board and resets the score and level stats.
         */
        void InitGame();
        
        /**
         * @brief Spawns the next queued Tetromino piece at the top of the board.
         */
        void SpawnPiece();
        
        /**
         * @brief Checks if a piece collides with the board boundaries or previously placed blocks.
         * @param piece The piece type identifier index.
         * @param rotation The piece's current rotation state.
         * @param posX The X coordinate on the board.
         * @param posY The Y coordinate on the board.
         * @return True if a collision is detected, otherwise false.
         */
        bool CheckCollision(int piece, int rotation, int posX, int posY);
        
        /**
         * @brief Locks the currently falling piece into the static board.
         */
        void LockPiece();
        
        /**
         * @brief Clears any completely filled rows and shifts the board downwards.
         */
        void ClearLines();
        
        /**
         * @brief Renders the game board, boundaries, score HUD, and current/next pieces.
         */
        void DrawBoard();
        
        /**
         * @brief Calculates a 1D index from 2D coordinates mapped to the specified rotation.
         * @param px The block's local X coordinate.
         * @param py The block's local Y coordinate.
         * @param r The rotation angle index.
         * @return The flattened 1D array index containing block data.
         */
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