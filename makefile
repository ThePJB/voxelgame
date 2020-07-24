
CC=gcc

src = $(wildcard src/*.c)
src += $(wildcard src/chunk/*.c)
obj = $(src:src/%.c=obj/%.o)
headers = $(wildcard inc/*.h)

# -std=c11 was not happy with stb ds
CFLAGS = -O3 -g -Wall -Werror=implicit-int -Werror=implicit-function-declaration -Werror=int-conversion -Werror=incompatible-pointer-types -Werror=return-type -Wno-missing-braces -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value  -Iinc -I/usr/include/freetype2 -fno-math-errno
LDFLAGS = -lGL -lglfw -ldl -lm -lfreetype

# obj depends on corresponding c and all headers
obj/%.o: src/%.c $(headers)
	$(CC) -c -o $@ $< $(CFLAGS)

voxelgame: $(obj)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

profile: $(src)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -pg && ./profile --profile && gprof profile gmon.out

.PHONY: clean
clean:
	rm -f $(obj) voxelgame
