# ----------------------------
# Makefile Options
# ----------------------------

NAME = CLOCK
ICON = icon.png
DESCRIPTION = "Digital/analog clock for the TI-84 Plus CE"

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz


# ----------------------------

include $(shell cedev-config --makefile)

