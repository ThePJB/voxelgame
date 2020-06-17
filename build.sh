gcc -o voxelgame main.c glad.c camera.c shader.c texture.c graphics.c -lGL -lglfw -ldl -lm -g -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-but-set-variable -std=c11 -Iinc

# -lX11 -lpthread -lXrandr -lXi