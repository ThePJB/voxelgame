#include "event.h"

event_callback callback_table[NUM_EVENTS] = {0};

void post_event(event_tag e, void* args) {
    if (callback_table[e] != 0) {
        callback_table[e](args);
    }
}

void register_listener(event_tag on_event, void (callback)(void* args)) {
    callback_table[on_event] = callback;
}