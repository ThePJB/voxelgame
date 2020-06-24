#ifndef EVENT_H
#define EVENT_H

#include "world.h"
#include <cglm/struct.h>


// so this should be extensible to entities and networking and stuff

typedef enum {
    EVENT_MOVE,
    EVENT_JUMP,
    EVENT_LOOK,
    EVENT_PLACE_BLOCK,
    EVENT_BREAK_BLOCK,
    EVENT_SELECT_BLOCK,
    NUM_EVENTS,
} event_tag;

typedef void (*event_callback)(void* args) ;

void post_event(event_tag e, void* args);
void register_listener(event_tag on_event, event_callback callback);    // should this be one to one or many to one


#endif