#include <SFML/Audio.hpp>
#include "audio_manager.hpp"
#include <iostream>
#include <optional>

#include "assets/click_mp3.h"
#include "assets/drop_mp3.h"
#include "assets/line_clear_mp3.h"
#include "assets/rotation_mp3.h"
#include "assets/background_mp3.h"
#include "assets/game_over_mp3.h"
#include "assets/main_menu_mp3.h"

namespace core
{
    namespace AudioManager
    {
        static sf::Music g_bgm;
        static bool g_initialized = false;

        static sf::SoundBuffer g_clickBuffer;
        static sf::SoundBuffer g_dropBuffer;
        static sf::SoundBuffer g_lineClearBuffer;
        static sf::SoundBuffer g_rotationBuffer;

        static std::optional<sf::Sound> g_clickSound;
        static std::optional<sf::Sound> g_dropSound;
        static std::optional<sf::Sound> g_lineClearSound;
        static std::optional<sf::Sound> g_rotationSound;

        void Init()
        {
            if (g_clickBuffer.loadFromMemory(assets_click_mp3, assets_click_mp3_len)) g_clickSound.emplace(g_clickBuffer);
            if (g_dropBuffer.loadFromMemory(assets_drop_mp3, assets_drop_mp3_len)) g_dropSound.emplace(g_dropBuffer);
            if (g_lineClearBuffer.loadFromMemory(assets_line_clear_mp3, assets_line_clear_mp3_len)) g_lineClearSound.emplace(g_lineClearBuffer);
            if (g_rotationBuffer.loadFromMemory(assets_rotation_mp3, assets_rotation_mp3_len)) g_rotationSound.emplace(g_rotationBuffer);

            g_initialized = true;
        }

        void PlayMenuMusic()
        {
            if (!g_initialized) return;
            if (g_bgm.openFromMemory(assets_main_menu_mp3, assets_main_menu_mp3_len)) {
                g_bgm.setLooping(true);
                g_bgm.play();
            }
        }

        void PlayGameMusic()
        {
            if (!g_initialized) return;
            if (g_bgm.openFromMemory(assets_background_mp3, assets_background_mp3_len)) {
                g_bgm.setLooping(true);
                g_bgm.play();
            }
        }

        void PlayGameOverMusic()
        {
            if (!g_initialized) return;
            if (g_bgm.openFromMemory(assets_game_over_mp3, assets_game_over_mp3_len)) {
                g_bgm.setLooping(false);
                g_bgm.play();
            }
        }

        void StopMusic()
        {
            if (!g_initialized) return;
            g_bgm.stop();
        }
        
        void SetMusicVolume(float volume)
        {
            if (!g_initialized) return;
            g_bgm.setVolume(volume);
        }
        
        float GetMusicVolume()
        {
            if (!g_initialized) return 100.f;
            return g_bgm.getVolume();
        }

        void PlayClickSound() { if (g_initialized && g_clickSound) g_clickSound->play(); }
        void PlayDropSound() { if (g_initialized && g_dropSound) g_dropSound->play(); }
        void PlayLineClearSound() { if (g_initialized && g_lineClearSound) g_lineClearSound->play(); }
        void PlayRotationSound() { if (g_initialized && g_rotationSound) g_rotationSound->play(); }

        void Shutdown()
        {
            if (!g_initialized) return;
            g_bgm.stop();
            g_initialized = false;
        }
    }
}