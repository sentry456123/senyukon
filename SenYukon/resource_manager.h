#pragma once

#include <memory>
#include <vector>

#include "raylib.h"

class ResourceManager {
    std::vector<char> zip_image;
    
public:
    Sound load_sound(const char *resource_path);
    void unload_sound(Sound sound);

    ResourceManager();

public:
	static void startup_singleton();
	static void shutdown_singleton();
    static ResourceManager *get_singleton();

private:
    static inline std::unique_ptr<ResourceManager> singleton;
};