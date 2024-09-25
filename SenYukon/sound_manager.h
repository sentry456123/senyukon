#pragma once

#include <unordered_map>
#include <string>

#include "raylib.h"

class SoundManager {
    std::unordered_map<std::string, Sound> sounds;

public:
    void play_sound(const char *filepath);
    ~SoundManager();

public:
    static void startup_singleton();
    static void shutdown_singleton();
    static SoundManager *get_singleton();

private:
    static inline SoundManager *singleton = nullptr;
};
