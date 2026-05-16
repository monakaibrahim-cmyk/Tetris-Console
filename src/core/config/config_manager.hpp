#pragma once

#include <string>

namespace core {

struct KeyBindings {
    int keyLeft;
    int keyRight;
    int keySoftDrop;
    int keyRotate;
    int keyHardDrop;
    int keyPause;
    int keyQuit;
    int musicVolume;
};

class ConfigManager {
public:
    ConfigManager(const std::string& filepath);
    void Load(KeyBindings& bindings);
    void Save(const KeyBindings& bindings);

private:
    std::string m_filepath;
    void SetDefaults(KeyBindings& bindings);
};

} // namespace core