#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

/*
    Function: house_init(struct House* h)
    Purpose:  Initializes a House struct with default values and allocated hunter storage.
    Params:
        Input/Output: struct House* h - house being initialized.
    Return: void
*/
void house_init(struct House* h) {
    h->room_count = 0;
    h->starting_room = NULL;
    
    h->hunter_capacity = 4;
    h->hunter_count = 0;
    h->hunters = malloc(h->hunter_capacity * sizeof(struct Hunter));
    
    h->caseFile.collected = 0;
    h->caseFile.solved = false;
    sem_init(&h->caseFile.mutex, 0, 1);
    
    h->ghost.has_exited = false;
}

/*
    Function: house_add_hunter(struct House* h, const char* hunter_name, int hunter_id)
    Purpose:  Adds a new Hunter to the house, resizing storage if needed.
    Params:
        Input/Output: struct House* h - house to add a hunter into.
        Input: const char* hunter_name - name of the hunter.
        Input: int hunter_id - ID assigned to the hunter.
    Return: void
*/
void house_add_hunter(struct House* h, const char* hunter_name, int hunter_id) {
    if(h->hunter_count >= h->hunter_capacity) {
        h->hunter_capacity *= 2;
        struct Hunter* new_array = realloc(h->hunters, h->hunter_capacity * sizeof(struct Hunter));
        if(!new_array) return;
        h->hunters = new_array;
    }
    
    hunter_init(&h->hunters[h->hunter_count], hunter_name, hunter_id, h->starting_room, &h->caseFile);
    h->hunter_count++;
}

/*
    Function: house_cleanup(struct House* h)
    Purpose:  Frees all dynamic memory and cleans up rooms, hunters, and synchronization primitives.
    Params:
        Input/Output: struct House* h - house being cleaned up.
    Return: void
*/
void house_cleanup(struct House* h) {
    for(int i = 0; i < h->room_count; i++) {
        room_cleanup(&h->rooms[i]);
    }
    
    for(int i = 0; i < h->hunter_count; i++) {
        hunter_cleanup(&h->hunters[i]);
    }
    
    free(h->hunters);
    sem_destroy(&h->caseFile.mutex);
}
