#pragma once

#include <string>

struct KeyBindings {
    int keyLeft;
    int keyRight;
    int keySoftDrop;
    int keyRotate;
    int keyHardDrop;
    int keyPause;
    int keyQuit;
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