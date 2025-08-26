#include <gbdk/platform.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "systemdetect.h"
#include "module_vwf.h"
#include "screen.h"

#include "filebrowser.h"

// debug:
// #include <gbdk/emu_debug.h>
// #include "pff_stub.h"
// #include "txt_player.h"

void main(void) {
    detect_system();
    setup_system();

    INIT_module_screen();
    INIT_module_vwf();
    INIT_module_misc_assets();

    while (true) {
        file_browser_execute();
        // debug:
        // extern FATFS filesystem; FATFS tmp = {.fsize=7337}; filesystem = tmp; // fake file size
        // txt_play_file("whatever2"); // direct test
    }
}