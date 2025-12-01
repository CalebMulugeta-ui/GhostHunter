#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

/*
    Function: ghost_init(struct Ghost* g, struct House* h)
    Purpose:  Initializes a Ghost struct with a random type, starting room, and default stats.
    Params:
        Input: struct House* h - contains all rooms and metadata needed for initialization.
        Input/Output: struct Ghost* g - the ghost instance being set up.
    Return: void
*/
void ghost_init(struct Ghost* g, struct House* h) {
    g->id = DEFAULT_GHOST_ID;
    
    const enum GhostType* ghost_types = NULL;
    int type_count = get_all_ghost_types(&ghost_types);
    int rand_idx = rand_int_threadsafe(0, type_count);
    g->type = ghost_types[rand_idx];
    
    rand_idx = rand_int_threadsafe(1, h->room_count);
    g->current_room = &h->rooms[rand_idx];
    g->current_room->ghost = g;
    
    g->boredom = 0;
    g->has_exited = false;
    
    log_ghost_init(g->id, g->current_room->name, g->type);
}

/*
    Function: ghost_update_stats(struct Ghost* g)
    Purpose:  Updates the ghost’s boredom level depending on whether hunters are present.
    Params:
        Input/Output: struct Ghost* g - ghost whose boredom counter is modified.
    Return: void
*/
void ghost_update_stats(struct Ghost* g) {
    if(room_has_hunters(g->current_room)) {
        g->boredom = 0;
    } else {
        g->boredom++;
    }
}

/*
    Function: ghost_check_exit(struct Ghost* g)
    Purpose:  Checks whether a ghost should exit due to excessive boredom.
    Params:
        Input/Output: struct Ghost* g - ghost that may transition to 'exited' state.
    Return: bool - true if ghost exited, false otherwise.
*/
bool ghost_check_exit(struct Ghost* g) {
    if(g->boredom > ENTITY_BOREDOM_MAX) {
        g->has_exited = true;
        
        log_ghost_exit(g->id, g->boredom, g->current_room->name);
        
        sem_wait(&g->current_room->mutex);
        g->current_room->ghost = NULL;
        sem_post(&g->current_room->mutex);
        
        return true;
    }
    
    return false;
}

/*
    Function: ghost_leave_evidence(struct Ghost* g)
    Purpose:  Randomly selects a valid evidence type for the ghost and places it in its current room.
    Params:
        Input: struct Ghost* g - ghost that may leave evidence.
    Return: void
*/
void ghost_leave_evidence(struct Ghost* g) {
    const enum EvidenceType* all_ev = NULL;
    int total = get_all_evidence_types(&all_ev);
    
    enum EvidenceType ghost_ev[3];
    int ev_count = 0;
    
    for(int i = 0; i < total; i++) {
        if(g->type & all_ev[i]) {
            ghost_ev[ev_count] = all_ev[i];
            ev_count++;
        }
    }
    
    if(ev_count > 0) {
        int rand_idx = rand_int_threadsafe(0, ev_count);
        enum EvidenceType ev_to_leave = ghost_ev[rand_idx];
        
        sem_wait(&g->current_room->mutex);
        evidence_set(&g->current_room->evidence, ev_to_leave);
        sem_post(&g->current_room->mutex);
        
        log_ghost_evidence(g->id, g->boredom, g->current_room->name, ev_to_leave);
    }
}

/*
    Function: ghost_move(struct Ghost* g)
    Purpose:  Moves the ghost to a random connected room if no hunters are present.
    Params:
        Input/Output: struct Ghost* g - ghost whose room will be changed.
    Return: void
*/
void ghost_move(struct Ghost* g) {
    if(room_has_hunters(g->current_room)) return;
    if(g->current_room->connection_count == 0) return;
    
    struct Room* from = g->current_room;
    int rand_idx = rand_int_threadsafe(0, from->connection_count);
    struct Room* target = from->connections[rand_idx];

    struct Room* first = (from < target) ? from : target;
    struct Room* second = (from < target) ? target : from;

    sem_wait(&first->mutex);
    sem_wait(&second->mutex);
    
    from->ghost = NULL;
    g->current_room = target;
    target->ghost = g;

    sem_post(&second->mutex);
    sem_post(&first->mutex);
    
    log_ghost_move(g->id, g->boredom, from->name, target->name);
}

/*
    Function: ghost_take_action(struct Ghost* g)
    Purpose:  Randomly chooses and performs one of the ghost’s actions: idle, leave evidence, or move.
    Params:
        Input/Output: struct Ghost* g - ghost that will perform an action.
    Return: void
*/
void ghost_take_action(struct Ghost* g) {
    int action = rand_int_threadsafe(0, 3);
    
    if(action == 0) {
        log_ghost_idle(g->id, g->boredom, g->current_room->name);
    } else if(action == 1) {
        ghost_leave_evidence(g);
    } else {
        ghost_move(g);
    }
}

/*
    Function: ghost_thread(void* data)
    Purpose:  Main thread loop controlling ghost behavior until it exits.
    Params:
        Input: void* data - pointer to a Ghost struct for this thread.
    Return: void* - always NULL after completion.
*/
void* ghost_thread(void* data) {
    struct Ghost* g = (struct Ghost*)data;
    
    while(!g->has_exited) {
        ghost_update_stats(g);
        
        if(!ghost_check_exit(g)) {
            ghost_take_action(g);
        }
    }
    
    return NULL;
}
