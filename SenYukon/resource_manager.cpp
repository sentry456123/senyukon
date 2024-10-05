#include "resource_manager.h"

#include <cassert>
#include <fstream>

#include "miniz.h"

static Sound LoadSoundFromMemory(const char *file_type, const void *data, int size) {
    Wave wave = LoadWaveFromMemory(file_type, (unsigned char *)data, size);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static std::vector<char> read_file(const char *filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}

static void *extract_zip_to_heap(const void *zip_image, size_t zip_image_size, const char *file_name, size_t *extracted_size) {
    mz_zip_archive zip_archive{};

    mz_zip_reader_init_mem(&zip_archive, zip_image, zip_image_size, 0);
    void *p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_name, extracted_size, 0);
    mz_zip_reader_end(&zip_archive);

    return p;
}

Sound ResourceManager::load_sound(const char *resource_path) {
    size_t extracted_size{};
    void *data = extract_zip_to_heap(zip_image.data(), zip_image.size(), resource_path, &extracted_size);
    Sound sound = LoadSoundFromMemory(GetFileExtension(resource_path), data, extracted_size);
    free(data);
    return sound;
}

void ResourceManager::unload_sound(Sound sound) {
    UnloadSound(sound);
}

ResourceManager::ResourceManager() {
    zip_image = read_file("res");
}

void ResourceManager::startup_singleton() {
    singleton = std::make_unique<ResourceManager>();
}

void ResourceManager::shutdown_singleton() {
    singleton.reset();
}

ResourceManager *ResourceManager::get_singleton() {
    assert(singleton);
    return singleton.get();
}
