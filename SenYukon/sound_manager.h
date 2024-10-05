#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "raylib.h"

class ResourceManager;

class SoundManager {
    std::unordered_map<std::string, Sound> sounds;
    ResourceManager *resource_manager;

public:
    void play_sound(const char *filepath);
    SoundManager(ResourceManager *resource_manager);
    ~SoundManager();

public:
    static void startup_singleton(ResourceManager *resource_manager);
    static void shutdown_singleton();
    static SoundManager *get_singleton();

private:
    static inline std::unique_ptr<SoundManager> singleton;
};
