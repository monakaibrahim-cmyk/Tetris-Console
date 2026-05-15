#pragma once

#include <cstdint>

#define DISCORD_CLIENT_ID 1504736262165364796

namespace core
{
    namespace DiscordManager
    {
        void Init();
        void SetActivity(const char* state, const char* details, const char* large_image_key, const char* large_image_text);
        void Shutdown();
    }
}