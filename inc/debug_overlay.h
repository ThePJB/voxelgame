#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H
/*
#include "camera.h"
#include "util.h"
#include "chunk_common.h"
#include "text.h"
#include "window.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"

#include "graphics.h"
#include "text.h"
#include "window.h"
#include "stb_ds.h"

#include "debug_overlay.h"

#include "chunk_common.h"

void draw_debug_info(float dt, camera cam, window_context *wc, chunk_manager *cm);

#endif