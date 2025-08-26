#ifndef _ENCODINGS_H_
#define _ENCODINGS_H_ 

#include "txt_enum.h"

extern TXT_RESULT copy_line2buf_ISOLatin1(uint32_t ofs);

// Copies a line at offset 'ofs' into text_buffer, interpreting the file as UTF-8
extern TXT_RESULT copy_line2buf_UTF8(uint32_t ofs);

// Copies a line at offset 'ofs' from UTF-8 file into text_buffer, mapping to ISO-8859-1 single bytes
extern TXT_RESULT copy_line2buf_UTF8_to_ISOLatin1(uint32_t ofs);


extern TXT_RESULT copy_line2buf_ISOLatin1_to_WINDOWS1252(uint32_t ofs);

// Copies a line from UTF-8 file at offset 'ofs' into text_buffer,
// mapping to single‑byte Windows‑1252 when possible.
extern TXT_RESULT copy_line2buf_UTF8_to_WINDOWS1252(uint32_t ofs);

// #define copy_line2buf	copy_line2buf_ISOLatin1
// #define copy_line2buf	copy_line2buf_UTF8
// #define copy_line2buf	copy_line2buf_UTF8_to_ISOLatin1
// #define copy_line2buf	copy_line2buf_UTF8_to_WINDOWS1252
extern TXT_RESULT (*copy_line2buf)(uint32_t ofs);
extern void set_encoding(uint8_t encoding);
extern void next_encoding(void);

extern uint8_t current_encoding;

#define DEFAULT_ENCODING TXT_ENCODING_UTF8


#endif // !_ENCODINGS_H_