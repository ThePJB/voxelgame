
CC=gcc

src = $(wildcard src/*.c)
obj = $(src:src/%.c=obj/%.o)
headers = $(wildcard inc/*.h)

CFLAGS = -O2 -g -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-but-set-variable -std=c11 -Iinc -I/usr/include/freetype2
LDFLAGS = -lGL -lglfw -ldl -lm -lfreetype

# obj depends on corresponding c and all headers
obj/%.o: src/%.c $(headers)
	$(CC) -c -o $@ $< $(CFLAGS)

voxelgame: $(obj)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) voxelgame
