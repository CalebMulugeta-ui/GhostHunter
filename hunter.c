#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

void hunter_init(struct Hunter* h, const char* h_name, int h_id, struct Room* start_room, struct CaseFile* cf) {
    strncpy(h->name, h_name, MAX_HUNTER_NAME - 1);
    h->name[MAX_HUNTER_NAME - 1] = '\0';
    
    h->id = h_id;
    h->current_room = start_room;
    h->casefile = cf;
    
    const enum EvidenceType* ev_types = NULL;
    int type_count = get_all_evidence_types(&ev_types);
    int rand_idx = rand_int_threadsafe(0, type_count);
    h->device = ev_types[rand_idx];
    
    roomstack_init(&h->path);
    
    h->fear = 0;
    h->boredom = 0;
    h->should_exit = false;
    h->return_to_van = false;
    h->exit_reason = LR_BORED;
    
    log_hunter_init(h_id, start_room->name, h_name, h->device);
}

bool hunter_move(struct Hunter* h, struct Room* target) {
    struct Room* from = h->current_room;
    
    struct Room* first = (from < target) ? from : target;
    struct Room* second = (from < target) ? target : from;
    
    sem_wait(&first->mutex);
    sem_wait(&second->mutex);
    
    if(target->hunter_count >= MAX_ROOM_OCCUPANCY) {
        sem_post(&second->mutex);
        sem_post(&first->mutex);
        return false;
    }
    
    room_remove_hunter(from, h);
    h->current_room = target;
    room_add_hunter(target, h);
    
    sem_post(&second->mutex);
    sem_post(&first->mutex);
    
    log_move(h->id, h->boredom, h->fear, from->name, target->name, h->device);
    return true;
}

void hunter_update_stats(struct Hunter* h) {
    sem_wait(&h->current_room->mutex);
    bool ghost_here = (h->current_room->ghost != NULL);
    sem_post(&h->current_room->mutex);
    
    if(ghost_here) {
        h->boredom = 0;
        h->fear++;
    } else {
        h->boredom++;
    }
}

void hunter_check_van(struct Hunter* h) {
    if(!h->current_room->is_exit) return;
    
    roomstack_clear(&h->path);
    
    if(h->return_to_van) {
        h->return_to_van = false;
        log_return_to_van(h->id, h->boredom, h->fear, h->current_room->name, h->device, false);
    }
    
    sem_wait(&h->casefile->mutex);
    
    if(evidence_has_three_unique(h->casefile->collected) &&
       evidence_is_valid_ghost(h->casefile->collected)) {
        h->casefile->solved = true;
        sem_post(&h->casefile->mutex);
        
        sem_wait(&h->current_room->mutex);
        room_remove_hunter(h->current_room, h);
        sem_post(&h->current_room->mutex);
        
        h->should_exit = true;
        h->exit_reason = LR_EVIDENCE;
        log_exit(h->id, h->boredom, h->fear, h->current_room->name, h->device, LR_EVIDENCE);
        return;
    }
    
    sem_post(&h->casefile->mutex);
    
    enum EvidenceType old = h->device;
    const enum EvidenceType* ev_types = NULL;
    int count = get_all_evidence_types(&ev_types);
    int rand_idx = rand_int_threadsafe(0, count);
    h->device = ev_types[rand_idx];
    
    log_swap(h->id, h->boredom, h->fear, old, h->device);
}

void hunter_check_exit_conditions(struct Hunter* h) {
    if(h->boredom > ENTITY_BOREDOM_MAX) {
        sem_wait(&h->current_room->mutex);
        room_remove_hunter(h->current_room, h);
        sem_post(&h->current_room->mutex);
        
        h->should_exit = true;
        h->exit_reason = LR_BORED;
        log_exit(h->id, h->boredom, h->fear, h->current_room->name, h->device, LR_BORED);
        return;
    }
    
    if(h->fear > HUNTER_FEAR_MAX) {
        sem_wait(&h->current_room->mutex);
        room_remove_hunter(h->current_room, h);
        sem_post(&h->current_room->mutex);
        
        h->should_exit = true;
        h->exit_reason = LR_AFRAID;
        log_exit(h->id, h->boredom, h->fear, h->current_room->name, h->device, LR_AFRAID);
        return;
    }
}

void hunter_gather_evidence(struct Hunter* h) {
    if(h->current_room->is_exit) return;

    sem_wait(&h->current_room->mutex);
    bool found_evidence = evidence_has(h->current_room->evidence, h->device);
    
    if(found_evidence) {
        evidence_clear(&h->current_room->evidence, h->device);
        sem_post(&h->current_room->mutex);
        
        sem_wait(&h->casefile->mutex);
        evidence_set(&h->casefile->collected, h->device);
        sem_post(&h->casefile->mutex);
        
        log_evidence(h->id, h->boredom, h->fear, h->current_room->name, h->device);
        
        if(!h->current_room->is_exit) {
            h->return_to_van = true;
            log_return_to_van(h->id, h->boredom, h->fear, h->current_room->name, h->device, true);
        }
    } else {
        sem_post(&h->current_room->mutex);
        
        int chance = rand_int_threadsafe(0, 100);
        if(chance < 10) {
            h->return_to_van = true;
            log_return_to_van(h->id, h->boredom, h->fear, h->current_room->name, h->device, true);
        }
    }
}

void hunter_choose_move(struct Hunter* h) {
    struct Room* target = NULL;
    struct Room* old = h->current_room;
    
    if(h->return_to_van) {
        target = roomstack_pop(&h->path);
        if(!target) return;
    } else {
        if(h->current_room->connection_count == 0) return;
        int rand_idx = rand_int_threadsafe(0, h->current_room->connection_count);
        target = h->current_room->connections[rand_idx];
    }
    
    bool moved = hunter_move(h, target);
    
    if(moved && !h->return_to_van) {
        roomstack_push(&h->path, old);
    }
}

void hunter_cleanup(struct Hunter* h) {
    roomstack_cleanup(&h->path);
}

void* hunter_thread(void* data) {
    struct Hunter* h = (struct Hunter*)data;

    while(!h->should_exit) {
        hunter_update_stats(h);
        hunter_check_van(h);

        if(!h->should_exit) {
            hunter_check_exit_conditions(h);
        }

        if(!h->should_exit) {
            hunter_gather_evidence(h);
            hunter_choose_move(h);
        }
    }
    
    return NULL;
}