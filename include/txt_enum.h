#ifndef _TXT_ENUM_H_
#define _TXT_ENUM_H_

#include <stdint.h>
// TXT enums

typedef enum TXT_RESULT {
	TXT_OK = 0,
    TXT_READ_ERROR,
	// TXT_SIZE_LIMIT_ERROR, //TODO?
    // TXT_EOF, // Necessita?
	N_TXT_RESULTS
} TXT_RESULT;

// these are not really correct...
typedef enum TXT_ENCODINGS {
	TXT_ENCODING_ASCII = 0,
    TXT_ENCODING_UTF8,
    TXT_ENCODING_ISO8859_LATIN1, // WINDOWS1252 is kinda same...
    TXT_ENCODING_WINDOWS1252,
    TXT_ENCODING_CP1252,
    N_TXT_ENCODINGS
} TXT_ENCODINGS;

#endif // !_TXT_ENUM_H_