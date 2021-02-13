# List of all the board related files.
BOARDSRC = $(BOARD_PATH)/boards/K320_BOARD/board.c

# Required include directories
BOARDINC = $(BOARD_PATH)/boards/K320_BOARD

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
