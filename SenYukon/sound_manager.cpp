#include <cassert>

#include "sound_manager.h"
#include "resource_manager.h"

void SoundManager::play_sound(const char *resource_path) {
    if (!sounds.contains(resource_path)) {
        Sound sound = resource_manager->load_sound(resource_path);
        
        sounds.insert({resource_path, sound});
        PlaySound(sound);
        return;
    }

    Sound sound = sounds.find(resource_path)->second;
    PlaySound(sound);
}

SoundManager::SoundManager(ResourceManager *resource_manager)
    : resource_manager(resource_manager) {

}

SoundManager::~SoundManager() {
    for (auto &sound : sounds) {
        resource_manager->unload_sound(sound.second);
    }
}

void SoundManager::startup_singleton(ResourceManager *resource_manager) {
    singleton = std::make_unique<SoundManager>(resource_manager);
}

void SoundManager::shutdown_singleton() {
    singleton.reset();
}

SoundManager *SoundManager::get_singleton() {
    assert(singleton);
    return singleton.get();
}
