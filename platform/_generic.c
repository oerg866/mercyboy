#include <stdio.h>

#include "compat.h"

/*
 * Generic main / WinMain for almost all compilers out there.
 */

#if defined (_MSC_VER)

int APIENTRY WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main_default(__argc, __argv);
}

#else

int main(int argc, char *argv[]) {
    return main_default(argc, argv);
}

#endif
