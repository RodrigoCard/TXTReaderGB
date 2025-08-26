#include "txt_player.h"
#include "joy.h"
#include "screen.h"
#include "menus.h"
#include "misc_assets.h"
#include "globals.h"
#include "encodings.h"
#include <stdio.h>
#include <string.h>
#include "module_vwf.h"

#include <gbdk/emu_debug.h>
#define PRINT(...) EMU_printf(__VA_ARGS__)
// #define PRINT(...)

const uint8_t * const TXT_ERRORS[N_TXT_RESULTS] = {"Read Ok!", "Read error" };
const uint8_t * const TXT_ENCODING_NAMES[N_TXT_ENCODINGS] = {"default", "UTF8", "ISO88591", "WIN1252", "CP1252"};
const uint8_t * const TXT_FONT_NAMES[FONT_COUNT] = {"Font 6px", "Font 5px", "VWfont"};

uint32_t line_positions[DISPLAY_MAX_LINES];
uint32_t prev_line_pos = 0, next_line_pos = 0; // prev =top, next=bottom
uint8_t  line_top_index = 0;     // circular index for line_positions[]
uint8_t  display_line_count = 0; // how many lines on screen (often=DISPLAY_MAX_LINES but can be less)

uint8_t  buf[TXT_READ_BUFFER_SIZE];
uint8_t  has_more_lines_bottom = 0, has_more_lines_top = 0; // could be a flag
uint8_t current=0, prev=0; // displayed lines
// TODO: both as flags
uint8_t nav=0; // started navigation
uint8_t dirty=0; // need full redraw
uint16_t selected_line=0, selected_line_prev=0; //file scope

uint8_t buf_index = 1; //another circular buffer index for drawing lines
uint32_t file_pos = 0;
uint8_t current_font=0;

void reset_vars(void) { // navigation vars
	nav=0;
	dirty=0;
	selected_line=0;
	selected_line_prev=0;
	current=0, prev=0;
}

// TODO: Move elsewhere
const char* formatSize(uint32_t bytes) {
	static char size_str[16];
	char temp[10];
	if (bytes < 1024) {
		uint32_t n = bytes;
		uint8_t i = 0;
		
		do {
			temp[i++] = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		uint8_t j = 0;
		while (i > 0) { size_str[j++] = temp[--i]; }

		strcpy(size_str + j, " bytes"); // bytes suffix
	} else {
		// Usa shifts para conversão (1024 = 2^10)
		uint32_t whole = bytes >> 10;        // Equivalente a /1024
		uint32_t remainder = bytes & 0x3FF;  // Equivalente a %1024 (1023 = 0x3FF)
		
		// Calcula décimos: (remainder * 10 + 512) / 1024 usando shifts
		// = ((remainder << 3) + (remainder << 1) + 512) >> 10
		uint32_t tenths = ((remainder << 3) + (remainder << 1) + 512) >> 10;
		
		// carry-over adjust (ex: 1023 bytes)
		if (tenths >= 10) {
			whole++;
			tenths = 0;
		}
		
		uint32_t n = whole;
		uint8_t i = 0;
		
		do {
			temp[i++] = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		
		uint8_t pos = 0;
		while (i > 0) { size_str[pos++] = temp[--i]; }
		
		// Adiciona ponto decimal e décimo
		size_str[pos++] = '.';
		size_str[pos++] = '0' + tenths;
		
		// Adiciona sufixo KB
		strcpy(size_str + pos, "Kb");
	}
	return size_str;
}

// encoding and font on bottom
static void drawStatusBar(void) {
	PRINT("encoding:%u, font:%u\n", current_encoding, current_font);
	sprintf(text_buffer, ICON_SELECT " %s " ICON_START " %s", TXT_ENCODING_NAMES[current_encoding], TXT_FONT_NAMES[current_font]);
	vwf_activate_font(FONT_VARW);
	menu_text_out(0, TXT_FIELD_SIZE_H, DEVICE_SCREEN_WIDTH, STATUS_COLOR, ITEM_TEXT_CENTERED, text_buffer);
	vwf_activate_font(current_font);
}

static void next_font(void) {
	current_font=(current_font+1) % FONT_COUNT;
	vwf_activate_font(current_font);
	dirty=1;
	drawStatusBar();
}

void DrawTextArea(const uint8_t * title) {
	// empty field
	screen_clear_rect(0, 0, TXT_FIELD_SIZE_W, TXT_FIELD_SIZE_H, TXT_COLOR);
	// corners
	screen_set_tile_xy(         0,          1, CORNER_UL);
	screen_set_tile_xy(0 + TXT_FIELD_SIZE_W - 1,          1, CORNER_UR);
	screen_set_tile_xy(         0, 0 + TXT_FIELD_SIZE_H - 1, CORNER_DL);
	screen_set_tile_xy(0 + TXT_FIELD_SIZE_W - 1, 0 + TXT_FIELD_SIZE_H - 1, CORNER_DR);
	// title on top
	// TODO: is this cutting something?
	sprintf(text_buffer, ICON_DOCUMENT " %s - %s", title, (formatSize(filesystem.fsize)));

	vwf_activate_font(FONT_VARW);
	menu_text_out(0, 0, DEVICE_SCREEN_WIDTH, TITLE_COLOR, ITEM_TEXT_CENTERED, text_buffer);
	vwf_activate_font(current_font);
	drawStatusBar();
}

// Draw all lines in line_positions (circular index!)
void DrawLines(void) {
	for (uint8_t i = 0; i < display_line_count; i++) {
		uint8_t color = TXT_COLOR;
		if (nav && i==current) { // selected line
			if (current==0) continue; // draw later
			color = TXT_SELECTED_COLOR;
		}
		uint8_t circ_index = (line_top_index + i) % DISPLAY_MAX_LINES;
		copy_line2buf(line_positions[circ_index]);
		menu_text_out(TXT_FIELD_POS_X, TXT_FIELD_POS_Y+i, TXT_CONTENT_WIDTH, color, ITEM_DEFAULT, text_buffer);
	}
	if (nav && current==0) { // text_buffer will contain the selected top line text, reuse it on scroll
		uint32_t pos = line_positions[(line_top_index) % DISPLAY_MAX_LINES];
		copy_line2buf(pos); // circ_index = 0
		menu_text_out(TXT_FIELD_POS_X, TXT_FIELD_POS_Y, TXT_CONTENT_WIDTH, TXT_SELECTED_COLOR, ITEM_DEFAULT, text_buffer);
	}
}



TXT_RESULT read_lines_bottom(void) {
	has_more_lines_bottom = 0;
	has_more_lines_top = 1;
	selected_line++;
	dirty = 1;
	
	line_top_index = (line_top_index + 1) %DISPLAY_MAX_LINES; // inc base index
	buf_index = (line_top_index + current)%DISPLAY_MAX_LINES; // new line index
	prev_line_pos = line_positions[buf_index]; // save pos value from oposite side
	line_positions[buf_index] = next_line_pos; // now put the new pos value
	file_pos = next_line_pos; // start from here

	pf_lseek(file_pos); // go to start
	uint8_t num_lines = 1; // =1 may be optimized out
	uint16_t br = 0;
	do {
		// TODO: smaller buf if diff<TXT_READ_BUFFER_SIZE
		if (pf_read(buf, TXT_READ_BUFFER_SIZE, &br) != FR_OK) return TXT_READ_ERROR;
		if (br == 0) break; //EOF

		for (uint16_t i = 0; i < br; ++i, ++file_pos) {
			// PRINT("%c", buf[i]);
			if (buf[i] == '\n') {
				if (num_lines-- > 1) {
					buf_index = (buf_index+1) % DISPLAY_MAX_LINES;
					line_positions[buf_index] = file_pos + 1;
					// PRINT("endline\n");
				} else {
					// PRINT("has_more_lines_bottom\n");
					next_line_pos = file_pos + 1; // +1 to skip '\n'
					has_more_lines_bottom = 1;
					break;
				}
			}
		}
	} while (br == TXT_READ_BUFFER_SIZE && !has_more_lines_bottom);
	return TXT_OK;
}

TXT_RESULT read_lines_top(void) {
	has_more_lines_top = 0;
	has_more_lines_bottom = 1;
	selected_line--;
	dirty = 1;

	line_top_index = (uint8_t)((int8_t)line_top_index+DISPLAY_MAX_LINES-1) %DISPLAY_MAX_LINES; // dec base index
	buf_index = (line_top_index + current)%DISPLAY_MAX_LINES; // new line index
	next_line_pos = line_positions[buf_index]; // save pos value from oposite side
	line_positions[buf_index] = prev_line_pos; // now put the new pos value
	file_pos = prev_line_pos;  // start from here

	if(file_pos==0) { // start of file, do nothing
		// PRINT("start of file, do nothing\n");
		pf_lseek(file_pos); // seek to start
		return TXT_OK;
	}
	
	uint32_t dist_from_start = file_pos;
	uint32_t chunk;
	uint32_t seek_pos = file_pos;
	uint8_t found = 0;
	uint8_t num_lines = 1; // =1 may be optimized out
	uint16_t br = 0;

	do {
		chunk = (dist_from_start >= TXT_READ_BUFFER_SIZE) ? TXT_READ_BUFFER_SIZE : dist_from_start;
		// PRINT(" chunk %d dist_from_start %d file_pos %d \n", chunk, dist_from_start, file_pos);
		seek_pos -= chunk;
		dist_from_start-=chunk;
		pf_lseek(seek_pos); // go to start of chunk
		if (pf_read(buf, chunk, &br) != FR_OK) return TXT_READ_ERROR;

		int16_t i = (br-1);
		file_pos--;
		if (buf[i] == '\n') { i--; file_pos--; } // skip the first \n
		
		for (; i >= 0; --i, --file_pos) {
			// PRINT("'%c'\n", (uint8_t)buf[i]);

			if (buf[i] == '\n') {
				found = 1;
				if (num_lines-- > 1) {
					// PRINT("R endline\n");
					buf_index = (uint8_t)((int8_t)buf_index+DISPLAY_MAX_LINES-1) % DISPLAY_MAX_LINES;
					line_positions[buf_index] = file_pos + 1;
				} else {
					// PRINT("R has_more_lines_top\n");
					prev_line_pos = file_pos + 1;
					has_more_lines_top = (file_pos>0);
					break;
				}
			}
		}
		// PRINT("endfor\n");
	} while (seek_pos > 0 && !has_more_lines_top && !found);
	// PRINT("seek_pos=%u, found=%d, hasmrlinestop=%d\n", (uint32_t)seek_pos, found, has_more_lines_top);
	if (!found) {
		// PRINT("Ended chunk but found no \\n\n");
		if(seek_pos==0) {
			// PRINT("seek_pos=0\n");
			prev_line_pos = 0;
			has_more_lines_top = 1;
		}
	}
	return TXT_OK;
}


// Reads and shows a loaded txt file, reads more data on demand
TXT_RESULT txt_play_file(const uint8_t * filepath, const uint8_t * filename) {
	PRINT("Reading: '%s'\n\t (%s)\n", filepath, filename);
	// TODO: save path for later, as it will be overriden in text_buffer
	// filebrowser.c:83: char last_filename[13] = "\0";
	if (pf_open(filepath) != FR_OK) { return TXT_READ_ERROR; }

	set_encoding(current_encoding);
	
	line_top_index = 0;
	display_line_count = 0;
	prev_line_pos = 0;
	next_line_pos = 0;
	has_more_lines_bottom = 0;
	
	line_positions[0] = 0; //first line
	display_line_count++;
	
	file_pos = 0;
	buf_index = 1; //another circular buffer index for drawing lines
	
	uint16_t br = 0;
	do { //Read enough of the first lines of the file so you can show it for the first time
		 // it basically seeks and saves '\n'+1 positions up to DISPLAY_MAX_LINES
		if (pf_read(buf, TXT_READ_BUFFER_SIZE, &br) != FR_OK) return TXT_READ_ERROR;
		if (br == 0) break; //EOF

		for (uint16_t i = 0; i < br; ++i, ++file_pos) {
			if (buf[i] == '\n') {
				if (display_line_count < DISPLAY_MAX_LINES) { //there are visible lines available
					display_line_count++;
					line_positions[buf_index] = file_pos + 1;
					buf_index = (line_top_index + display_line_count) % DISPLAY_MAX_LINES;
				} else { //full buffer, save next pos for later
					next_line_pos = file_pos + 1;
					has_more_lines_bottom = 1;
					break;
				}
			}
		}
	} while (br == TXT_READ_BUFFER_SIZE && !has_more_lines_bottom);

	// PRINT("display_line_count %u has_more_lines_bottom %u\n", (uint16_t)display_line_count, (uint8_t)has_more_lines_bottom);

	// Now draw everything you have
	DrawTextArea(filename);
	DrawLines();
	BGP_DEFAULT;
	
	// Navigation loop
	reset_vars();
	while (1) {
		vsync();
		PROCESS_INPUT();
		
		if (KEY_PRESSED(J_DOWN)) {
			PRINT("Press DOWN\t");
			if (current + 1 < display_line_count) {
				prev=current; current++; selected_line++;
			} else if(has_more_lines_bottom) { // So, move lines up
				if (read_lines_bottom() != TXT_OK) return TXT_READ_ERROR;
			} else if (!nav) {
				selected_line = 0;
				current = 0;
			}
			// PRINT("V selected %u top_idx %u buf_idx %u, pl %u nl %u fp %u\n", (uint16_t)selected_line, (uint8_t)line_top_index, (uint8_t)buf_index, (uint32_t)prev_line_pos, (uint32_t)next_line_pos, (uint32_t)file_pos);
		} else if (KEY_PRESSED(J_UP)) {
			PRINT("Press UP\t");
			if (current > 0) {
				prev=current; current--; selected_line--;
			} else if(has_more_lines_top) { // So, move lines down
				if (read_lines_top() != TXT_OK) return TXT_READ_ERROR;
				
			} else if (!nav) { // select bottom line
				selected_line = display_line_count-1;
				prev=current;
				current = selected_line;
			}
			// PRINT("^ selected %u top_idx %u buf_idx %u, pl %u nl %u fp %u\n", (uint16_t)selected_line, (uint8_t)line_top_index, (uint8_t)buf_index, (uint32_t)prev_line_pos, (uint32_t)next_line_pos, (uint32_t)file_pos);
		} else if (KEY_PRESSED(J_RIGHT)) {
			if (current + 1 < display_line_count) {
				selected_line+= display_line_count-current;
				prev=current;
				current=display_line_count-(display_line_count>0);
			} else if (!nav) {
				selected_line = display_line_count-1;
				prev=current;
				current = selected_line;
			} else {
				uint8_t line_counter = DISPLAY_MAX_LINES;
				BGP_LOADING;
				while(--line_counter && has_more_lines_bottom) {
					if (read_lines_bottom() != TXT_OK) return TXT_READ_ERROR;
				}
				vsync();
				BGP_DEFAULT;
			}
			// PRINT("> selected %u top_idx %u buf_idx %u, pl %u nl %u fp %u\n", (uint16_t)selected_line, (uint8_t)line_top_index, (uint8_t)buf_index, (uint32_t)prev_line_pos, (uint32_t)next_line_pos, (uint32_t)file_pos);
		} else if (KEY_PRESSED(J_LEFT)) {
			if (current > 0) {
				selected_line-=current;
				prev=current;
				current=0;
			} else if (!nav) {
				selected_line = 0;
				prev=current-1;//doesnt matter, just need to be different
				current = 0;
			} else {
				uint8_t line_counter = DISPLAY_MAX_LINES;
				BGP_LOADING;
				while(--line_counter && has_more_lines_top) {
					if (read_lines_top() != TXT_OK) return TXT_READ_ERROR;
				}
				vsync();
				BGP_DEFAULT;
			}
			// PRINT("< selected %u top_idx %u buf_idx %u, pl %u nl %u fp %u\n", (uint16_t)selected_line, (uint8_t)line_top_index, (uint8_t)buf_index, (uint32_t)prev_line_pos, (uint32_t)next_line_pos, (uint32_t)file_pos);
		} else if (KEY_PRESSED(J_B)) { // sair com B
			break;
		} else if (KEY_PRESSED(J_SELECT)) { // encodings
			next_encoding();
			BGP_LOADING;
			dirty = 1;
			nav = 0;
		} else if (KEY_PRESSED(J_START)) { // font
			BGP_LOADING;
			next_font(); // sets dirty internally
			nav = 0;
		}
		
		if (current!=prev) {
			// PRINT("current!=prev\n");
			if (nav) {
				menu_text_out(TXT_FIELD_POS_X, TXT_FIELD_POS_Y+prev, TXT_CONTENT_WIDTH, TXT_COLOR, ITEM_DEFAULT, text_buffer);
			} else {
				nav = 1;
				if (selected_line == 1) {
					selected_line = 0;
					current = 0;
				}
				// PRINT("NAV 1 D sel_line=0, cur=0\n");
			}
			copy_line2buf(line_positions[(line_top_index+current) % DISPLAY_MAX_LINES]);
			menu_text_out(TXT_FIELD_POS_X, TXT_FIELD_POS_Y+current, TXT_CONTENT_WIDTH, TXT_SELECTED_COLOR, ITEM_DEFAULT, text_buffer);
			prev=current;

		} else if (dirty) {
			PRINT("dirty=1, redrawing\n");
			dirty = 0;
			drawStatusBar();
			DrawLines();
			BGP_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK );
		}
	}
	reset_vars();
		
	return TXT_OK;
}

