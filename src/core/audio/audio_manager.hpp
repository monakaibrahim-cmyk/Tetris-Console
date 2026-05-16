#pragma once
#include <string>

namespace core
{
    namespace AudioManager
    {
        void Init();
        void PlayMenuMusic();
        void PlayGameMusic();
        void PlayGameOverMusic();
        void StopMusic();
        void SetMusicVolume(float volume);
        float GetMusicVolume();
        
        void PlayClickSound();
        void PlayDropSound();
        void PlayLineClearSound();
        void PlayRotationSound();
        void Shutdown();
    }
}