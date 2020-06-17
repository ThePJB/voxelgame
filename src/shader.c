#include <stdlib.h>
#include <stdio.h>
#include "glad.h"

char *slurp(const char *path) {
    char *buffer = 0;
    long length;
    FILE * f = fopen (path, "rb");

    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = calloc(length + 1, 1);
        if (buffer) {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }

    if (!buffer) {
        printf("problem loading file %s\n", path);
        exit(1);
    }
    return buffer;
}

unsigned int make_shader(char *path, unsigned int type) {
    char *src = slurp(path);

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar * const*)&src, NULL);
    glCompileShader(shader);
    int success = 0;
    char buf[512] = {0};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, buf);
        printf("error compiling %s: %s\n", path, buf);
        exit(1);
    }

    return shader;
}