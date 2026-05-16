#include "config_manager.hpp"
#include <fstream>
#include <ncurses.h>

namespace core
{

    ConfigManager::ConfigManager(const std::string& filepath) : m_filepath(filepath)
    {
        //
    }

    void ConfigManager::SetDefaults(KeyBindings& bindings)
    {
        bindings.keyLeft = KEY_LEFT;
        bindings.keyRight = KEY_RIGHT;
        bindings.keySoftDrop = KEY_DOWN;
        bindings.keyRotate = KEY_UP;
        bindings.keyHardDrop = ' ';
        bindings.keyPause = 'p';
        bindings.keyQuit = 'q';
        bindings.musicVolume = 100;
    }

    void ConfigManager::Load(KeyBindings& bindings)
    {
        std::ifstream file(m_filepath);

        if (!file.is_open())
        {
            SetDefaults(bindings);
            Save(bindings);
            return;
        }

        std::string line;

        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '[' || line[0] == ';')
            {
                continue;
            }

            size_t delimiterPos = line.find('=');

            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);

                try
                {
                    int value = std::stoi(line.substr(delimiterPos + 1));
                    
                    if (key == "Left")
                    {
                        bindings.keyLeft = value;
                    }
                    else if (key == "Right")
                    {
                        bindings.keyRight = value;
                    }
                    else if (key == "SoftDrop")
                    {
                        bindings.keySoftDrop = value;
                    }
                    else if (key == "Rotate")
                    {
                        bindings.keyRotate = value;
                    }
                    else if (key == "HardDrop")
                    {
                        bindings.keyHardDrop = value;
                    }
                    else if (key == "Pause")
                    {
                        bindings.keyPause = value;
                    }
                    else if (key == "Quit")
                    {
                        bindings.keyQuit = value;
                    }
                    else if (key == "MusicVolume")
                    {
                        bindings.musicVolume = value;
                    }
                }
                catch (...)
                {
                    // Ignore invalid (corrupted) config values, rely on defaults
                }
            }
        }
    }

    void ConfigManager::Save(const KeyBindings& bindings)
    {
        std::ofstream file(m_filepath);

        if (file.is_open())
        {
            file << "[Controls]\n";
            file << "Left=" << bindings.keyLeft << "\n";
            file << "Right=" << bindings.keyRight << "\n";
            file << "SoftDrop=" << bindings.keySoftDrop << "\n";
            file << "Rotate=" << bindings.keyRotate << "\n";
            file << "HardDrop=" << bindings.keyHardDrop << "\n";
            file << "Pause=" << bindings.keyPause << "\n";
            file << "Quit=" << bindings.keyQuit << "\n";
            file << "MusicVolume=" << bindings.musicVolume << "\n";
        }
    }

} // namespace core