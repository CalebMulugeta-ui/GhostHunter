#include <string.h>
#include "defs.h"
#include "helpers.h"

void room_init(struct Room* r, const char* room_name, bool exit_room) {
    strncpy(r->name, room_name, MAX_ROOM_NAME - 1);
    r->name[MAX_ROOM_NAME - 1] = '\0';

    r->connection_count = 0;
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        r->connections[i] = NULL;
    }

    r->ghost = NULL;
    r->hunter_count = 0;
    for(int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        r->hunters[i] = NULL;
    }

    r->is_exit = exit_room;
    r->evidence = 0;
    sem_init(&r->mutex, 0, 1);
}

void room_connect(struct Room* room1, struct Room* room2) {
    if(room1->connection_count < MAX_CONNECTIONS) {
        room1->connections[room1->connection_count] = room2;
        room1->connection_count++;
    }
    
    if(room2->connection_count < MAX_CONNECTIONS) {
        room2->connections[room2->connection_count] = room1;
        room2->connection_count++;
    }
}

void room_add_evidence(struct Room* r, enum EvidenceType ev) {
    evidence_set(&r->evidence, ev);
}

bool room_add_hunter(struct Room* r, struct Hunter* h) {
    if(r->hunter_count >= MAX_ROOM_OCCUPANCY) return false;
    
    r->hunters[r->hunter_count] = h;
    r->hunter_count++;
    return true;
}

void room_remove_hunter(struct Room* r, struct Hunter* h) {
    for(int idx = 0; idx < r->hunter_count; idx++) {
        if(r->hunters[idx] == h) {
            for(int j = idx; j < r->hunter_count - 1; j++) {
                r->hunters[j] = r->hunters[j + 1];
            }
            r->hunters[r->hunter_count - 1] = NULL;
            r->hunter_count--;
            return;
        }
    }
}

bool room_has_hunters(struct Room* r) {
    sem_wait(&r->mutex);
    bool has_hunters = (r->hunter_count > 0);
    sem_post(&r->mutex);
    return has_hunters;
}

void room_cleanup(struct Room* r) {
    sem_destroy(&r->mutex);
}