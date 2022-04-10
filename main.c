#include <stdio.h>

#include "compat.h"

// Handle the need for WinMain for MSC/MSVC.

#if defined (_MSC_VER) || defined (_MSVC_VER)

int APIENTRY WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main_default(__argc, __argv);
}

#else

int main(int argc, char *argv[]) {
    return main_default(argc, argv);
}

#endif
