#include "compat.h"
#include "trace.h"

#include <stdio.h>
#include <string.h>

/*
    This is a WinMain implementation for really, *really* old versions of Microsoft Visual C++.
    The problem with these is that they only provide the lpCmdLine parameter, but no
    __argc/__argv globals to get the command line parameters out. So we manually split the commandline.
*/

char proc_file_name[260+1]; // Process file name, 256 characters + dot + 3 extension + null terminator
char **_mb_argv = NULL;
int _mb_argc = 0;

void process_args_and_set_argc(char *in) {
/*
    Process an input string and inserts a 0x00 character after every argument
*/
    bool in_quotes = false;

    while (*in) {
        if (in_quotes && *in != '\"') {
            ++in;
            continue;
        }

        if (*in == '\"') {
            in_quotes = !in_quotes;
            *in = 0x00;
            _mb_argc++;
        } else if (*in == ' ') {
            *in = 0x00;
            while (isspace(*in)) { in++; }
            _mb_argc++;
        } else {
            in++;

            // Final character in string, so we count it
            if (*in == 0x00) {
                _mb_argc++;
            }

        }
    } // while (*in)
}

void set_argv(char *in) {
/*
    Populates the global _mb_argv with pointers to the separate arguments after preparing the input string.
 */
    char **dst;
    int i;

    _mb_argv = alloc_mem((_mb_argc + 1 + 1) * sizeof(char*)); // 1 extra for module file name

    _mb_argv[0] = proc_file_name; // First argument is the file name of the application, lpCmdLine does not contain it

    dst = &_mb_argv[1];

    for(i = 0; i < _mb_argc; i++) {
        *dst = in;
        dst++;

        while (*in) { in++; }
        while (isspace(*in)) { in++; } // This argument is done, find next argument by skipping whitespace
    }

    *dst = NULL; // in argv the element after the last one is always NULL.
    _mb_argc++; // Since we had to add the module file name to the start

}

int APIENTRY WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    process_args_and_set_argc(lpCmdLine);
    set_argv(lpCmdLine);

    GetModuleFileName(NULL, proc_file_name, sizeof(proc_file_name));

    /* Doing it this way because there's no console on these old compilers ... */

    if (_mb_argc < 2) {
        MessageBox(NULL,
            "MercyBoy - Portable Gameboy Emulator -- Version "
            VERSION_STRING "\n"
            "Written by Eric Voirin -- http://github.com/oerg866/mercyboy\n"
            "USAGE: mercyboy.exe <romfile>",
            "MercyBoy",
            MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }

    return main_default(_mb_argc, _mb_argv);
}
