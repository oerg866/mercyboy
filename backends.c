#include "backends.h"
#include "bendlist.h"
#include "trace.h"

#include <stdio.h>

/* Template magic in C! Ha ha ...
 * Iterates through backends of arbitrary nature and tries to find one by name if given */
#define ITERATE_BACKENDS(x, name) \
    unsigned i;\
    for (i = 0; i < sizeof(x) / sizeof(*x); i++) { \
        backend = x[i]; \
        print_msg("Backend %s, available: %d\n", backend->name, backend->present); \
        if ((name != NULL && backend->present && strcmp(backend->name, name) == 0) \
         || (name == NULL && backend->present)) { \
            print_msg("Using backend %s.\n", backend->name); \
            return backend; \
        } \
    }

audio_backend_t *get_audio_backend(const char *name) {
    audio_backend_t *backend = NULL;

    ITERATE_BACKENDS(audio_backends, name);

    print_msg("ERROR: No backends available.\n");
    return NULL;
}

video_backend_t *get_video_backend(const char *name) {
    video_backend_t *backend = NULL;

    ITERATE_BACKENDS(video_backends, name);

    print_msg("ERROR: No backends available.\n");
    return NULL;
}

input_backend_t *get_input_backend(const char *name) {
    input_backend_t *backend = NULL;

    ITERATE_BACKENDS(input_backends, name);

    print_msg("ERROR: No backends available.\n");
    return NULL;
}
