# Compiler and flags useful -fverbose-asm -Os -o - , 
# http://web.archive.org/web/20130528172555/http://www.ibm.com/developerworks/library/l-graphvis/
# https://stackoverflow.com/questions/517589/tools-to-get-a-pictorial-function-call-graph-of-code?noredirect=1&lq=1
# intel vtune is shit prefer linux perf 
#
#  -fsanitize=address -fsanitize=leak
# strict aliasing not always very good but in general it can enable some optimization it might break code https://stackoverflow.com/questions/1225741/performance-impact-of-fno-strict-aliasing

CC = gcc
CFLAGS = -ggdb3 -O3 -DVK_USE_PLATFORM_WAYLAND_KHR -std=c99 -Wall -Wextra -pedantic -save-temps -Wshadow -Wfloat-equal -Wpointer-arith -Wstrict-prototypes -Wformat=2 -Wcast-align -fsanitize=undefined   -ftree-vectorize -ftree-loop-vectorize -fopt-info-vec-missed -fopt-info-vec-optimized -fstrict-aliasing

LIBS = -lglfw -lvulkan

# Output binary
OUT = main

# Source files
SRC = main.c 

# Build target
$(OUT): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -o $(OUT)

# Clean target
clean:
	rm -f $(OUT)

# Phony targets
.PHONY: clean

