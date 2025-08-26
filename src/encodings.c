#include "encodings.h"
#include "globals.h"
#include "txt_player.h"

TXT_RESULT (*copy_line2buf)(uint32_t ofs);
uint8_t current_encoding = DEFAULT_ENCODING;

#define CHAR_UNKNOWN '\x0b'

void set_encoding(uint8_t encoding) {
	current_encoding = encoding;
	switch (current_encoding) {
        default:
		case 0:
            copy_line2buf = &copy_line2buf_ISOLatin1;
            break;
		case 1:
			copy_line2buf = &copy_line2buf_UTF8;
			break;
        case 2:
			copy_line2buf = &copy_line2buf_UTF8_to_ISOLatin1;
			break;
		case 3:
			copy_line2buf = &copy_line2buf_UTF8_to_WINDOWS1252;
			break;
		case 4:
			copy_line2buf = &copy_line2buf_ISOLatin1_to_WINDOWS1252;
			break;
	}
}

void next_encoding(void) {
	set_encoding(((current_encoding+1) % N_TXT_ENCODINGS));
}

TXT_RESULT copy_line2buf_ISOLatin1(uint32_t ofs) {
	pf_lseek(ofs);
	uint16_t br, idx = 0;
	do {
		if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
		if (br == 0 || buf[0] == '\n' || idx >= DISPLAY_MAX_LINE_LEN - 1) break;
		if (buf[0] == '\r') continue; // filter properly later
		text_buffer[idx++] = buf[0];
	} while (1);
	text_buffer[idx] = '\0';
	return TXT_OK;
}

// Copies a line at offset 'ofs' into text_buffer, interpreting the file as UTF-8
TXT_RESULT copy_line2buf_UTF8(uint32_t ofs) {
    pf_lseek(ofs);
    uint16_t br, idx = 0;
    do {
        if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
        if (br == 0 || buf[0] == '\n' || idx >= DISPLAY_MAX_LINE_LEN - 1) break;
        if (buf[0] == '\r') continue; // ignore CR

        uint8_t c = buf[0];
        if (c < 0x80) {
            // ASCII
            text_buffer[idx++] = c;
        } else {
            // Multi-byte UTF-8
            uint8_t seq_len;
            if ((c & 0xE0) == 0xC0)       seq_len = 2;
            else if ((c & 0xF0) == 0xE0)  seq_len = 3;
            else if ((c & 0xF8) == 0xF0)  seq_len = 4;
            else continue; // Invalid byte, skip

            // Check if there's space in buffer for full sequence
            if (idx + seq_len >= DISPLAY_MAX_LINE_LEN) break;

            // text_buffer[idx++] = c;

            for (uint8_t i = 1; i < seq_len; ++i) {
                if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
                if (br == 0) break;
                uint8_t cc = buf[0];
                if ((cc & 0xC0) != 0x80) break; // Invalid continuation byte
                text_buffer[idx++] = cc;
            }
        }
    } while (1);

    text_buffer[idx] = '\0';
    return TXT_OK;
}

// Copies a line at offset 'ofs' from UTF-8 file into text_buffer, mapping to ISO-8859-1 single bytes
TXT_RESULT copy_line2buf_UTF8_to_ISOLatin1(uint32_t ofs) {
    pf_lseek(ofs);
    uint16_t br, idx = 0;
    do {
        if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
        if (br == 0 || buf[0] == '\n' || idx >= DISPLAY_MAX_LINE_LEN - 1) break;
        if (buf[0] == '\r') continue;

        uint8_t c = buf[0];
        uint32_t codepoint = 0;

        if (c < 0x80) {
            // ASCII (direct mapping)
            text_buffer[idx++] = c;
        } else {
            // Determine UTF-8 sequence length
            uint8_t seq_len = 0;
            if ((c & 0xE0) == 0xC0)       seq_len = 2;
            else if ((c & 0xF0) == 0xE0)  seq_len = 3;
            else if ((c & 0xF8) == 0xF0)  seq_len = 4;
            else continue; // Invalid first byte

            // Read remaining bytes
            uint8_t buf_seq[4] = { c };
            for (uint8_t i = 1; i < seq_len; ++i) {
                if (pf_read(buf, 1, &br) != FR_OK || br == 0) goto end;
                if ((buf[0] & 0xC0) != 0x80) goto end; // Invalid continuation byte
                buf_seq[i] = buf[0];
            }

            // Decode codepoint from UTF-8 sequence
            if (seq_len == 2) {
                codepoint = ((buf_seq[0] & 0x1F) << 6) | (buf_seq[1] & 0x3F);
            } else if (seq_len == 3) {
                codepoint = ((buf_seq[0] & 0x0F) << 12) |
                            ((buf_seq[1] & 0x3F) << 6) |
                            (buf_seq[2] & 0x3F);
            } else if (seq_len == 4) {
                codepoint = ((buf_seq[0] & 0x07) << 18) |
                            ((buf_seq[1] & 0x3F) << 12) |
                            ((buf_seq[2] & 0x3F) << 6) |
                            (buf_seq[3] & 0x3F);
            }

            // Map to ISO-8859-1 if possible
            if (codepoint <= 0xFF) {
                text_buffer[idx++] = (uint8_t)codepoint;
            } else {
                text_buffer[idx++] = CHAR_UNKNOWN; // Replace unsupported chars
            }
        }
    } while (1);

end:
    text_buffer[idx] = '\0';
    return TXT_OK;
}

TXT_RESULT copy_line2buf_ISOLatin1_to_WINDOWS1252(uint32_t ofs) {
    pf_lseek(ofs);
    uint16_t br, idx = 0;

    do {
        if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
        if (br == 0 || buf[0] == '\n' || idx >= DISPLAY_MAX_LINE_LEN - 1) break;
        if (buf[0] == '\r') continue;

        uint8_t c = buf[0];

        // In ISO-8859-1, 0x00–0xFF are valid single-byte characters.
        // But 0x80–0x9F are control characters in ISO-8859-1,
        // while in Windows-1252 they are printable symbols.
        switch (c) {
            case 0x80: text_buffer[idx++] = 0x80; break; // €
            case 0x82: text_buffer[idx++] = 0x82; break; // ‚
            case 0x83: text_buffer[idx++] = 0x83; break; // ƒ
            case 0x84: text_buffer[idx++] = 0x84; break; // „
            case 0x85: text_buffer[idx++] = 0x85; break; // …
            case 0x86: text_buffer[idx++] = 0x86; break; // †
            case 0x87: text_buffer[idx++] = 0x87; break; // ‡
            case 0x88: text_buffer[idx++] = 0x88; break; // ˆ
            case 0x89: text_buffer[idx++] = 0x89; break; // ‰
            case 0x8A: text_buffer[idx++] = 0x8A; break; // Š
            case 0x8B: text_buffer[idx++] = 0x8B; break; // ‹
            case 0x8C: text_buffer[idx++] = 0x8C; break; // Œ
            case 0x8E: text_buffer[idx++] = 0x8E; break; // Ž
            case 0x91: text_buffer[idx++] = 0x91; break; // ‘
            case 0x92: text_buffer[idx++] = 0x92; break; // ’
            case 0x93: text_buffer[idx++] = 0x93; break; // “
            case 0x94: text_buffer[idx++] = 0x94; break; // ”
            case 0x95: text_buffer[idx++] = 0x95; break; // •
            case 0x96: text_buffer[idx++] = 0x96; break; // –
            case 0x97: text_buffer[idx++] = 0x97; break; // —
            case 0x98: text_buffer[idx++] = 0x98; break; // ˜
            case 0x99: text_buffer[idx++] = 0x99; break; // ™
            case 0x9A: text_buffer[idx++] = 0x9A; break; // š
            case 0x9B: text_buffer[idx++] = 0x9B; break; // ›
            case 0x9C: text_buffer[idx++] = 0x9C; break; // œ
            case 0x9E: text_buffer[idx++] = 0x9E; break; // ž
            case 0x9F: text_buffer[idx++] = 0x9F; break; // Ÿ

            // You can also remap unhandled control characters here if needed.
            default:
                // For 0x00–0x7F and 0xA0–0xFF: copy directly
                // For 0x81, 0x8D, 0x8F, 0x90, 0x9D: not used in CP1252 – replace with CHAR_UNKNOWN
                if ((c >= 0x20 && c <= 0x7E) || (c >= 0xA0)) {
                    text_buffer[idx++] = c;
                } else if (c < 0x20 || (c >= 0x7F && c <= 0x9F)) {
                    text_buffer[idx++] = CHAR_UNKNOWN;
                } else {
                    text_buffer[idx++] = c;
                }
                break;
        }
    } while (1);

    text_buffer[idx] = '\0';
    return TXT_OK;
}

// Copies a line from UTF-8 file at offset 'ofs' into text_buffer,
// mapping to single‑byte Windows‑1252 when possible.
TXT_RESULT copy_line2buf_UTF8_to_WINDOWS1252(uint32_t ofs) {
    pf_lseek(ofs);
    uint16_t br, idx = 0;
    do {
        if (pf_read(buf, 1, &br) != FR_OK) return TXT_READ_ERROR;
        if (br == 0 || buf[0] == '\n' || idx >= DISPLAY_MAX_LINE_LEN - 1) break;
        if (buf[0] == '\r') continue;

        uint8_t c = buf[0];
        uint32_t codepoint = 0;

        if (c < 0x80) {
            // ASCII range directly maps
            text_buffer[idx++] = c;
        } else {
            // Determine UTF-8 sequence length
            uint8_t seq_len = 0;
            if ((c & 0xE0) == 0xC0)      seq_len = 2;
            else if ((c & 0xF0) == 0xE0) seq_len = 3;
            else if ((c & 0xF8) == 0xF0) seq_len = 4;
            else continue; // skip invalid byte

            uint8_t seq[4];
            seq[0] = c;
            for (uint8_t i = 1; i < seq_len; ++i) {
                if (pf_read(buf, 1, &br) != FR_OK || br == 0) goto done;
                if ((buf[0] & 0xC0) != 0x80) goto done;
                seq[i] = buf[0];
            }

            // Decode Unicode code point
            if (seq_len == 2) {
                codepoint = ((seq[0] & 0x1F) << 6) | (seq[1] & 0x3F);
            } else if (seq_len == 3) {
                codepoint = ((seq[0] & 0x0F) << 12)
                          | ((seq[1] & 0x3F) << 6)
                          | (seq[2] & 0x3F);
            } else if (seq_len == 4) {
                codepoint = ((seq[0] & 0x07) << 18)
                          | ((seq[1] & 0x3F) << 12)
                          | ((seq[2] & 0x3F) << 6)
                          | (seq[3] & 0x3F);
            }

            // Map Unicode code point to CP‑1252 if possible:
            // Windows‑1252 extends ISO‑8859‑1 in the 0x80–0x9F range
            if (codepoint < 0x80) {
                text_buffer[idx++] = (uint8_t)codepoint;
            }
            else if (codepoint >= 0xA0 && codepoint <= 0xFF) {
                // Latin‑1 compatible block
                text_buffer[idx++] = (uint8_t)codepoint;
            }
            else if (codepoint == 0x20AC) {
                // Euro sign (€) maps to 0x80 in CP‑1252
                text_buffer[idx++] = 0x80;
            }
            else {
                // Some other CP‑1252-specific characters:
                switch (codepoint) {
                    case 0x201A: text_buffer[idx++] = 0x82; break;
                    case 0x0192: text_buffer[idx++] = 0x83; break;
                    case 0x201E: text_buffer[idx++] = 0x84; break;
                    case 0x2026: text_buffer[idx++] = 0x85; break;
                    case 0x2020: text_buffer[idx++] = 0x86; break;
                    case 0x2021: text_buffer[idx++] = 0x87; break;
                    case 0x02C6: text_buffer[idx++] = 0x88; break;
                    case 0x2030: text_buffer[idx++] = 0x89; break;
                    case 0x0160: text_buffer[idx++] = 0x8A; break;
                    case 0x2039: text_buffer[idx++] = 0x8B; break;
                    case 0x0152: text_buffer[idx++] = 0x8C; break;
                    case 0x0161: text_buffer[idx++] = 0x9A; break;
                    case 0x203A: text_buffer[idx++] = 0x9B; break;
                    case 0x0153: text_buffer[idx++] = 0x9C; break;
                    case 0x017E: text_buffer[idx++] = 0x9E; break;
                    case 0x0178: text_buffer[idx++] = 0x9F; break;
                    default:
                        text_buffer[idx++] = CHAR_UNKNOWN;
                        break;
                }
            }
        }
    } while (1);

done:
    text_buffer[idx] = '\0';
    return TXT_OK;
}