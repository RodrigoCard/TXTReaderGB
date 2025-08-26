#include "pff_stub.h"

#include <gbdk/emu_debug.h>

#include "txt_player.h"

const char * fake_file_contents = 
	"\tText Reader GB\n\n"
	"Hello Reader!\n\n"
	"This is a utility that\n"
	"reads text files from \n"
	"SD card.\n"
	"Compatible flashcarts:\n"
	" - EZ-Flash Junior;\n"
	" - Everdrive X;\n"
	" - Not compatible with\n"
	"  most cheap everdrive\n"
	"  clones, maybe newer \n"
	"  ones work\n\n"
	"This rom is just for\n"
	"demonstration purposes.\n"
	"If you have a compatible\n"
	"cart, get the correct\n"
	"rom n copy to your cart,\n"
	"with some text files.\n\n"
	"There is some sample txt\n"
	"files you can use inside\n"
	"the zip file.\n\n"
	"Follow on itch.io for\n"
	"future updates.\n\n"
	"Have fun!\n\n"
	"<end of file>"
	;
// const char * fake_file_contents = "1 This is a test.\n2 a\n3 b\n4FOUR\n5 The small fox jumped over something.\n6 this is another line.\n7...\n8>End  ";
// const char * fake_file_contents = "€Ÿ¡¿£™˜ƒª°®©™ >\nÀÃÁaáãàeéêiíoóôòõuúü.\nTEST\n\nanother line.\n  <End>  ";
// const char * fake_file_contents = "€ƒ˜™®Ÿ¡£©ª°¿™ÿŸ >\nÀÃÁaáãàeéêiíoóôòõuúü.\n	<End>";
// const char * fake_file_contents =
// 	">€ƒ\n\n\tTest\"file\"\n\nÀÃÁaáãàeéêiíoóôòõuúü.\n"
// 	"Hello\nolá\n¡Hola!\n€uro\n—dash\n"
// 	"fake_file_contents 001\naaaa\n"
// 	"fake_file_contents 002\naa\n"
// 	"fake_file_contents 003\naa\n"
// 	"fake_file_contents 004\naa\n"
// 	"fake_file_contents 005\naa aaaaaaaaaaa aaaaaaaaaa aaaaabbbb bbbb bbb bbb bbbbbbbbb. bb bbbbbb\n"
// 	"fake_file_contents 006\nccccccccc cccccccccc ccc cc ccccccccc cc cccccccccccddd dddd d dd d ddddd\n"
// 	"fake_file_contents 007\naa\n"
// 	"fake_file_contents 008\naa\n"
// 	"fake_file_contents 009\naa\n"
// 	"fake_file_contents 010\naa\n"
// 	"fake_file_contents 011\nbbbb\n"
// 	"fake_file_contents 012\nbb\n"
// 	"fake_file_contents 013\nbb\n"
// 	"fake_file_contents 014\nbb\n"
// 	"fake_file_contents 015\nbb\n"
// 	"fake_file_contents 016\nbb\n"
// 	"fake_file_contents 017\nbb\n"
// 	"fake_file_contents 018\nbb\n"
// 	"fake_file_contents 019\nbb\n"
// 	"fake_file_contents 020\nbb\n"
// 	"\n\n\n\t\t<End>";
size_t fake_file_pos = 0;

// Stub de pf_read que lê de fake_file_contents
FRESULT pf_read(void* buff, uint16_t btr, uint16_t* br) {
	// EMU_printf("pf_read: %s %d %d", buff, (uint16_t)btr, (uint16_t)*br);
	size_t remaining = strlen(fake_file_contents) - fake_file_pos;
	size_t to_copy  = (btr < remaining) ? btr : remaining;
	
	if (to_copy > 0) {
		memcpy(buff, fake_file_contents + fake_file_pos, to_copy);
		fake_file_pos += to_copy;
	}
	
	*br = to_copy;
	return FR_OK;
}

FRESULT pf_open(const char* path) {
	// EMU_printf("pf_open: %s", path);
	(void)path;
	fake_file_pos = 0;
	return FR_OK;
}

FRESULT pf_lseek(uint32_t ofs) {
	// EMU_printf("pf_lseek: %d", (uint32_t)ofs);
	size_t len = strlen(fake_file_contents);
	if (ofs > len) {
		fake_file_pos = len;
	} else {
		fake_file_pos = ofs;
		//EOF
	}
	return FR_OK;
}

