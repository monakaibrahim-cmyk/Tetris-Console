#pragma once

#include <string>

namespace core
{

/**
 * @brief Holds user-defined keyboard controls and audio preferences.
 */
struct KeyBindings
{
    int keyLeft;
    int keyRight;
    int keySoftDrop;
    int keyRotate;
    int keyHardDrop;
    int keyPause;
    int keyQuit;
    int musicVolume;
};

/**
 * @brief Manages loading and saving user configuration (e.g., key bindings and volume).
 */
class ConfigManager
{
public:
    /**
     * @brief Constructs a ConfigManager bound to a specific file.
     * @param filepath The path to the configuration file (e.g., "config.ini").
     */
    ConfigManager(const std::string& filepath);
    
    /**
     * @brief Loads bindings from the configuration file. Falls back to defaults on error.
     * @param bindings The KeyBindings struct to populate.
     */
    void Load(KeyBindings& bindings);
    
    /**
     * @brief Saves the current bindings to the configuration file.
     * @param bindings The KeyBindings struct to save.
     */
    void Save(const KeyBindings& bindings);

private:
    std::string m_filepath;
    
    /**
     * @brief Sets fallback default settings for all keys and volume.
     * @param bindings The KeyBindings struct to reset.
     */
    void SetDefaults(KeyBindings& bindings);
};

} // namespace core