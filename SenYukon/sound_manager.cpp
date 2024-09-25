#include <cassert>

#include "sound_manager.h"

void SoundManager::play_sound(const char *filepath) {
    if (!sounds.contains(filepath)) {
        Sound sound = LoadSound(filepath);
        sounds.insert({filepath, sound});
        PlaySound(sound);
        return;
    }

    Sound sound = sounds.find(filepath)->second;
    PlaySound(sound);
}

SoundManager::~SoundManager() {
    for (auto &sound : sounds) {
        UnloadSound(sound.second);
    }
}

void SoundManager::startup_singleton() {
    singleton = new SoundManager();
}

void SoundManager::shutdown_singleton() {
    delete singleton;
}

SoundManager *SoundManager::get_singleton() {
    assert(singleton);
    return singleton;
}
