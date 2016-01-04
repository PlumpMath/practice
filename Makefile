ALL_LIBS=
CFLAGS=-Wall -g -std=c11

check-syntax:
	clang -o /dev/null $(CFLAGS) $(ALL_LIBS) -S ${CHK_SOURCES}

tags:
	etags --declarations ./*.h ./*.c
