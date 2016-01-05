ALL_LIBS=
CFLAGS=-Wall -g -Ilib -Ilib/nanovg/src -std=c11

platform: lib/platform.c lib/platform.h
	clang $(CFLAGS) `pkg-config --cflags sdl2` `pkg-config --libs sdl2` -framework OpenGL -Llib/nanovg/build -lnanovg -o platform lib/platform.c

platform_test: lib/platform_test.c
	clang -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o platform_test.dylib lib/platform_test.c

# run as `make PRACTICE=1234-56-89 gui
# kinda janky but it works
gui: FORCE
	clang -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o ${PRACTICE}.dylib ${PRACTICE}.c

# can also just do make 1234-12-12 and skip this target.
cli: FORCE
	clang $(CFLAGS) -o ${PRACTICE} ${PRACTICE}.c

check-syntax:
	clang -o /dev/null $(CFLAGS) $(ALL_LIBS) -S ${CHK_SOURCES}

tags:
	etags --declarations ./*.h ./*.c

FORCE:
