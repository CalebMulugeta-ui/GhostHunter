#include <stdlib.h>
#include "defs.h"

void roomstack_init(struct RoomStack* s) {
    s->head = NULL;
}

void roomstack_push(struct RoomStack* s, struct Room* r) {
    struct RoomNode* new_node = malloc(sizeof(struct RoomNode));
    if(!new_node) return;
    
    new_node->room = r;
    new_node->next = s->head;
    s->head = new_node;
}

struct Room* roomstack_pop(struct RoomStack* s) {
    if(!s->head) return NULL;
    
    struct RoomNode* node = s->head;
    struct Room* room_ptr = node->room;
    s->head = node->next;
    
    free(node);
    return room_ptr;
}

bool roomstack_is_empty(struct RoomStack* s) {
    return s->head == NULL;
}

void roomstack_clear(struct RoomStack* s) {
    while(!roomstack_is_empty(s)) {
        roomstack_pop(s);
    }
}

void roomstack_cleanup(struct RoomStack* s) {
    roomstack_clear(s);
}