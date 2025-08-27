#ifndef _TXT_PLAYER_H_
#define _TXT_PLAYER_H_

#include <gbdk/platform.h>
#include <stdint.h>

#include "misc_assets.h"
#include "encodings.h"
#include "txt_enum.h"

// #define TXT_READ_EMULATION
#ifndef TXT_READ_EMULATION
#include "pff.h"
#else // TXT_READ_EMULATION
#include "pff_stub.h"
#endif // !TXT_READ_EMULATION

#define TXT_READ_BUFFER_SIZE	64
#define TXT_READ_BUFFER_SIZE2	256 // TODO: (A) to display full line on window
#define DISPLAY_MAX_LINES       14 //3
#define DISPLAY_MAX_LINE_LEN    25 //64
#define DISPLAY_MAX_LINE_WIDTH	(DEVICE_SCREEN_BUFFER_WIDTH-16) // whole screen width minus 2 tiles

#define TXT_FIELD_POS_X		1
#define TXT_FIELD_POS_Y		2
#define TXT_FIELD_SIZE_W	20 
#define TXT_FIELD_SIZE_H	17
#define TXT_CONTENT_WIDTH	18
#define TXT_CONTENT_HEIGHT	15 // not used?

#define TXT_COLOR			BLACK_ON_WHITE
#define TXT_SELECTED_COLOR	WHITE_ON_DK_GR
#define TITLE_COLOR			LT_GR_ON_BLACK
#define STATUS_COLOR		LT_GR_ON_BLACK

#define BGP_DEFAULT BGP_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK )
#define BGP_LOADING BGP_REG = DMG_PALETTE(DMG_LITE_GRAY, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK )

extern FATFS filesystem;

extern const uint8_t * const TXT_ERRORS[N_TXT_RESULTS];
extern const uint8_t * const TXT_ENCODING_NAMES[N_TXT_ENCODINGS];

extern uint8_t buf[TXT_READ_BUFFER_SIZE];
extern uint8_t current_font;

extern TXT_RESULT txt_play_file(const uint8_t * filepath, const uint8_t * filename);

#endif // !_TXT_PLAYER_H_
