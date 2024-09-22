#include "yukon.h"

int main(int argc, char *argv[]) {
    play();
}

#if _WIN32 && !_DEBUG

#include <Windows.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    play();
}

#endif
