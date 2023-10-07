#ifndef COLORS_H_
#define COLORS_H_

#define COLOR_STD	    "\033[39m"
#define COLOR_BLACK	    "\033[30m"
#define COLOR_WHITE	    "\033[97m"
#define COLOR_RED	    "\033[91m"
#define COLOR_GREEN	    "\033[92m"
#define COLOR_BLUE	    "\033[94m"
#define COLOR_PURPLE	"\033[95m"
#define COLOR_YELLOW	"\033[33m"

#define COLOR_RESET	    "\033[0m"

#define PAINT_TEXT(COLOR, TEXT) (COLOR TEXT COLOR_RESET)


#endif
