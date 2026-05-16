#include "discord_manager.hpp"
#include "discord.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

namespace core
{
    namespace DiscordManager
    {
        static discord::Core* g_core = nullptr;
        static std::thread g_thread;
        static std::atomic<bool> g_running{false};
        static std::mutex g_mutex;

        static void UpdateLoop()
        {
            while (g_running)
            {
                {
                    std::lock_guard<std::mutex> lock(g_mutex);
                    if (g_core)
                    {
                        g_core->RunCallbacks();
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }

        void Init()
        {
            auto result = discord::Core::Create(DISCORD_CLIENT_ID, static_cast<std::uint64_t>(discord::CreateFlags::NoRequireDiscord), &g_core);

            if (result != discord::Result::Ok || !g_core)
            {
                std::cerr << "Failed to instantiate Discord Core! Error code: " << static_cast<int>(result) << std::endl;
                return;
            }

            g_core->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
                std::cout << "Discord Log[" << static_cast<int>(level) << "]: " << message << std::endl;
            });

            g_running = true;
            g_thread = std::thread(UpdateLoop);
        }

        void SetActivity(const char* state, const char* details, const char* large_image_key, const char* large_image_text)
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (!g_core) return;

            discord::Activity activity{};
            if (state) activity.SetState(state);
            if (details) activity.SetDetails(details);
            if (large_image_key) activity.GetAssets().SetLargeImage(large_image_key);
            if (large_image_text) activity.GetAssets().SetLargeText(large_image_text);
            activity.SetType(discord::ActivityType::Playing);

            g_core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
                if (result != discord::Result::Ok)
                {
                    std::cerr << "Failed to update Discord activity! Error code: " << static_cast<int>(result) << std::endl;
                }
            });
        }

        void Shutdown()
        {
            g_running = false;
            
            if (g_thread.joinable())
            {
                g_thread.join();
            }

            std::lock_guard<std::mutex> lock(g_mutex);
            if (g_core)
            {
                delete g_core;
                g_core = nullptr;
            }
        }
    }
}
