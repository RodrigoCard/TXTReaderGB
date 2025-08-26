#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "vwf.h"
#include "module_vwf.h"

// graphic assets
#include "font_fixed6.h"
#include "font_fixed5.h"
#include "font_var.h"

#if defined(SEGA)
#define DMG_BLACK     0x03
#define DMG_DARK_GRAY 0x02
#define DMG_LITE_GRAY 0x01
#define DMG_WHITE     0x00
#endif

BANKREF(module_vwf)

// initialize the VWF subsystem
uint8_t INIT_module_vwf(void) BANKED {
    vwf_load_font(0, font_fixed6, BANK(font_fixed6));
    vwf_load_font(1, font_fixed5, BANK(font_fixed5));
    vwf_load_font(2, font_var, BANK(font_var));
    // vwf_load_font(3, font_var, BANK(font_var));
    vwf_activate_font(0);
    vwf_set_colors(DMG_BLACK, DMG_WHITE);
    return 0;
}