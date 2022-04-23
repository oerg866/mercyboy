# Building MercyBoy


## Planned build environments:

- Windows NT 3.x
- Windows 3.1 / Visual C++ 1.52
- MS-DOS / DJGPP or OpenWatcom
- Old Linux (SDL1)

## `make` hints

- Use `make USE_SDL2=0` to disable SDL2
- Use `make TARGET=WIN9X` to only build stuff that works on Windows 9x (i.e. no SDL2, etc)

## Linux / GCC (Any reasonably modern version I guess)

- Install the SDL2 libs (`apt install libsdl2-dev`)
- Change to the repository directory
- Run `make` or `make DEBUG=1`

## MacOS X (I don't own a mac but I'm told it works)

- Make sure libsdl2 is installed
- You're smart (and seemingly way too wealthy for your own good), you can figure it out.

## Windows / MinGW

- Change to the repository directory.
- Type `mingw32-make` or `mingw32-make DEBUG=1`
- A file named `mercyboy.exe` will be built.

You can also run `mingw32-make TARGET=WIN9X` to compile without SDL2 support. Depending on the MinGW version used you may need KernelEx to run the binary still. (I think MinGW 4.x was the last one to work here...)

## Windows / Visual C++ 4.2 (1996)

*This might work with other 4.x versions of MSVC++ but I have not tried...*

This compiler supports building for several different architectures. I don't know how well this works, but be aware that for now MercyBoy only works on Little Endian CPU architectures.

- Copy `MBOY98.MAK` from the `_vc42` platform directory into the root folder.
- Open `Microsoft Development Studio`
- Click `File` - `Open Workspace`
- On the file type filter, select `.mak` files.
- Select `MBOY98.MAK`
- Build the project as required. Output files will be in `MB_REL` or `MB_DEBUG` for release and debug configurations respectively.

**Note: This build only supports these backends:**

- `a_wvout`
- `v_gdi`
- `i_win32`
