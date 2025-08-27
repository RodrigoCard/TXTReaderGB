# TODO:
	- Incorrect width with short text in shorter fonts
		-- get line width for each line instead of DISPLAY_MAX_LINE_LEN
	- draw '>' at right side when more chars are not shown
	- for lines bigger than available width:
		- Show more when press A on a window
		- Blocker: needs to fix utf8 width calculation

	- encoding is ignored on width count! my utf8 is wrong
	-- is line width calculation working properly?
		- no, not handling multibyte correctly,
		- store the converted byte instead of converting everytime,
		  so you wont need changes on vwf calculations

	- read DISPLAY_MAX_LINE_LEN characters when seeking for \n, waste less sd reads
		-- or properly cache more text in ram
		-- talvez seja melhor so dar uma opcao de vwf font msm, já tem ate calculo de width pronto!
			- divide por 8 q tu tem o total de tiles! ou usa o valor direto pra comparar (144 de width para 18 tiles!)
	
	- some file/encoding combinations can corrupt bg tiles on long line
		- I suspect it is related to line width calculations being wrong.
	
	- gfx messy when opening files too fast while screen is drawing
		- keep holding A when opening the first file and the app will crash fast

# TODO Encodings:
	- Refactor UTF8: store pre-converted and keep it a single byte on buffer
		- 
	X Simple ISO-8859-1 Latin
		/ it is indeed Windows1252 (CP1252) (rename stuff)
	- UTF8 control chars
		X C2 Codepoint
		X Partial C3 Codepoint (CP1252 remapped)
		/ C3/C4 Codepoints
		X then the next byte
		X print a single character
	- skip_utf8_bom
	- more encodings


# TODO LATER:
	- Auto Line Break option
	- check correct encodings!
	- adjustable \t size
<<<<<<< HEAD
=======
	- a read somewhere there is everdrive emulation in some emulators, use that!
>>>>>>> master
	- File browser:
		- <- -> to select the next column
			- check page selection ( menu_execute > onTranslateKeyFileBrowser )
		- recent files
<<<<<<< HEAD
	- a read somewhere there is everdrive emulation in some emulators, use that!
=======
>>>>>>> master
	- try gg/sms 
		- vgm player worked correctly on my gg/sms cheap flashcarts, should be doable
 	- find a faster way to scroll instead of redrawing everything char by char

# DONE:
	X fix make clean
	X LEFT RIGHT to skip a screen (14 lines)
	X darken bg color to indicate it is busy (re)loading stuff
 	X show file sizes as KB if > 9999 bytes (remember: 1Kb = 1024b, not 1000b)
	X Move enconding funcs?
	X SELECT (on browser): show all extensions
	X change title filebrowser root: TXT Reader GB (VERSION)
	X quebre txt_play_file em mais funcoes didaticas
	X press right after nav to put cursor at bottom
	X File browser:
		X allow all extensions
		X B to back a directory
	X <?> character for unavailable characters utf8
	x SELECT: change encoding
	X START: change font to vwf/fixed
	x vwf font on top bar
	X vwf font on bottom bar
	X Encoding selection
	X Test files

# SAMPLE TEXT:
	/ Find some nice poems and other texts to include
		X pangrams in various languages
		X public domain stuff
		X Shakespear poems
		X Pt/Br poems
		- unicode explanation
		- curiosities
		- useful
			X game boy history
			- facts about game boy
			x combinations of buttons for GB paletes
			- some game guides and cheats
			- homebrew gaming recomendations
