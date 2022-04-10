# Backends

Backends provide the bridge between the user's host hardware and the emulated gameboy.

**Mercyboy** Uses the following backend tpyes

- Audio

  For handling audio playback.

- Video

  For handling rendering

- Input

  For handling button inputs to the virtual gameboy.

## General info

Backends largely reside in their own C file that gets compiled and linked against.

Backends are represented by a structure defined in `backends.h`, named as such: `x_backend_t` where `x` is the backend type (e.g. `video`).

All backend structure types have two members at the beginning:

- `const char *name`

  Set to the backend's internal name.

- `const uint8_t present`

  Set to 1 when the backend was included in compilation, 0 if not.

The source files for the backends also define an `extern x_backend_t` structure. When creating a new backend, a pointer to this structure must be added to `bendlist.h` to make this backend usable.

The backends' source files are prefixed with `y_` where `y` is the type of backend:

- `i_` for Input
- `a_` for Audio
- `v_` for Video

Example: `v_sdl2.c`

Backends are selected for compilation by a `DEFINE` prefixed with `VIDEO_` (e.g.`VIDEO_SDL2`). On most platforms this can be done using the `Makefile`.

Their functions are prefixed with its name, for example: `v_sdl2_init()`.

## Input backends

Input backends handle button inputs and forwards them to the emulator.

Input backends are represented by the structure `input_backend_t`.

### `input_backend_t`

This structure defines an audio backend. Aside from `name` and `present` all members are function pointers to functions the backend has to implement. At the time of writing, none of these parameters are optional.

- `name`

  See above.

- `present`

  See above.

- `init`

  Function pointer to `int (*) ()`

  Contains initialization code for this backend. Can be used for allocating required memory, initialize input hardware, etc.

- `deinit`

  Function pointer to `void (*) ()`

  Contains de-initialization code for this backend.

- `get_buttons`

  Function pointer to `uint8_t (*) ()`

  This function returns the current button state. The returned byte is a bit field that is defined as follows:

  | Bit | Button |
  |-----|--------|
  |  0  | A      |
  |  1  | B      |
  |  2  | Select |
  |  3  | Start  |
  |  4  | Right  |
  |  5  | Left   |
  |  6  | Up     |
  |  7  | Down   |

## Audio backends

This section describes audio backends, their instantiation and configuration.

Backends may (should (really should)) call `audio_do_timing(float seconds_per_buffer)` whenever a new buffer is processed to enable basing frame timing off the audio backend.

Audio backends are predestined to make use of the `ringbuffer` See RINGBUF.md for more info.

### Backend config (`struct audio_config`)

It is allowed for the backend initialization routines to change the values in this struct as requested audio parameters may not be supported by the underlying hardware or APIs.

- `uint16_t sample_rate` - The initially requested sample rate.
- `uint16_t bits_per_sample` - The initially requested bit depth (bits per sample).
- `uint16_t channels` - The initially requested amount of channels (1 for mono, 2 for stereo).
- `uint16_t buffer_size` - The buffer size **in SAMPLES, not bytes!**

  This is a guidance and may not necessarily be fitting to the underlying hardware, in which case the internal buffering of the emulator must be adapted to the necessary buffering scheme by the backend.

### `audio_backend_t`

This structure defines an audio backend. Aside from `name` and `present` all members are function pointers to functions the backend has to implement. At the time of writing, none of these parameters are optional.

- `name`

  See above.

- `present`

  See above.

- `init`

  Function pointer to `int (*) (audio_config *cfg)`

  Contains initialization code for this backend. Can be used for allocating required memory, initialize sound hardware, etc.

- `deinit`

  Function pointer to `void (*) ()`

  Contains de-initialization code for this backend.

- `play_buffer`

  Function pointer to `audio_buffer_status (*) (uint8_t *buffer, uint32_t length)`

  The function receives a buffer of `length` size **in bytes** which, depending on sample rate, ***is not a constant***. The backend's buffering must accommodate and buffer.

  This function is allowed to block. In this case, video frame timing will depend on the backend.

  The return type, `audio_buffer_status`, is an enum which can contain one of the following values:

  - `AUDIO_BUFFER_ERROR` - There was a problem playing back the buffer.
  - `AUDIO_BUFFER_TAKEN` - The buffer was taken over by the backend. **IT MUST BE DEALLOCATED BY THE BACKEND**.
  - `AUDIO_BUFFER_COPIED` - The buffer was copied by the backend and can be disposed. The emulator will free the buffer.

###

## Video backends

Input backends are represented by the structure `video_backend_t`.

Video backends manage an internal palette of **12** entries (Object palettes + Background palette each containing 4 colors).

### Backend config (`struct video_config`)

- `uint16_t screen_width` - The initially requested output window width.
- `uint16_t screen_height` - The initially requested output window height.
- `uint16_t bpp` - The initially requested bit depth (bits per pixel)
- `uint16_t use_audio_timing` - Specifies whether or not the audio backend's buffer callback timing should be used for the screen refresh.

  This parameter can be ignored by the backend.

This struct is not yet fully thought through and may change at any point.

### Helper struct `gameboy_palette`

Its only member, `uint8_t color[4]` contains the index values of a palette.

### `video_backend_t`

This structure defines a video backend. Aside from `name` and `present` all members are function pointers to functions the backend has to implement. At the time of writing, none of these parameters are optional.

- `name`

  See above.

- `present`

  See above.

- `init`

  Function pointer to `int (*) (video_config *cfg)`

  Contains initialization code for this backend. Can be used for allocating required memory, etc.

- `deinit`

  Function pointer to `void (*) ()`

  Contains de-initialization code for this backend.

- `update_palette`

  Function pointer to `void (*) (uint8_t pal_offset, gameboy_palette palette)`

  This function is called when the emulated gameboy updates the object or background palettes.

  `pal_offset` is the starting index of where the new palette entries go.
  `gameboy_palette` is the actual palette.

- `write line`

  Function pointer to `void (*) (int line, uint8_t *linebuf)`

  Writes a line of width `LCD_WIDTH` (defined in video.h) with the index `line` into the internal buffer.

  `linebuf` is a buffer of bytes containing indexes from 0-11 that the backend can use to get the appropriate color for its own framebuffer.

- `frame_done`

  Function pointer to `void (*) ()`

  Indicates to the backend that the frame should be presented on screen. Depending on timing method used in the actual emulator it is called more or less every 1/60th of a second.

- `event_handler`

  Function pointer to `video_backend_status (*) ()`.

  Handles events. Event handling is required for many types of UIs and video backends, these tasks can be done here.

  The return value indicates the status of the UI / backend.

  `video_backend_status` is an enum which can contain one of the following values:

  - `VIDEO_BACKEND_RUNNING` - The backend is running normally.
  - `VIDEO_BACKEND_EXIT` - The backend signals that the emulator should exit.
