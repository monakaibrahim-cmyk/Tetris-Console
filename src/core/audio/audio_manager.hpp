#pragma once
#include <string>

namespace core
{
    /**
     * @brief Manages audio playback, including background music and sound effects.
     */
    namespace AudioManager
    {
        /**
         * @brief Initializes the audio buffers and loads sound assets into memory.
         */
        void Init();
        
        /**
         * @brief Plays the background music for the main menu.
         */
        void PlayMenuMusic();
        
        /**
         * @brief Plays the background music for active gameplay.
         */
        void PlayGameMusic();
        
        /**
         * @brief Plays the game over track.
         */
        void PlayGameOverMusic();
        
        /**
         * @brief Stops the currently playing background music.
         */
        void StopMusic();
        
        /**
         * @brief Sets the volume for the background music.
         * @param volume The volume level (0.0 to 100.0).
         */
        void SetMusicVolume(float volume);
        
        /**
         * @brief Retrieves the current volume of the background music.
         * @return The volume level (0.0 to 100.0).
         */
        float GetMusicVolume();
        
        /** @brief Plays the UI click sound effect. */
        void PlayClickSound();
        
        /** @brief Plays the hard drop sound effect. */
        void PlayDropSound();
        
        /** @brief Plays the line clear sound effect. */
        void PlayLineClearSound();
        
        /** @brief Plays the piece rotation sound effect. */
        void PlayRotationSound();
        
        /**
         * @brief Shuts down the audio system and stops all active sounds.
         */
        void Shutdown();
    }
}