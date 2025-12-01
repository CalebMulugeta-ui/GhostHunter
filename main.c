#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

int main() {

    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

  printf("Ghost Hunt Simulator\n\n");
  struct House house;
  house_init(&house);

  house_populate_rooms(&house);
  printf("House initialized with %d rooms\n", house.room_count);

  ghost_init(&house.ghost, &house);
  printf("Ghost Initialized: %s in %s\n\n",
	 ghost_to_string(house.ghost.type),
	 house.ghost.current_room->name);

  printf("Enter hunter information (type 'done' when finished):\n");

  char name[MAX_HUNTER_NAME];
  int id;

  while(true){
    printf("Hunter name: ");
    if (scanf("%s", name) != 1) {
      break;
    }
        
    if (strcmp(name, "done") == 0) {
      break;
    }
        
    printf("ID: ");
    if (scanf("%d", &id) != 1) {
      break;
    }
        
    house_add_hunter(&house, name, id);
    printf("Added hunter: %s (ID: %d)\n\n", name, id);
  }  

  printf("\nStarting Game\n");
  printf("Hunters: %d\n", house.hunter_count);
  printf("Ghost: %s\n\n", ghost_to_string(house.ghost.type));

  pthread_t ghost_thread_id;
  pthread_t* hunter_threads = malloc(house.hunter_count * sizeof(pthread_t));

  pthread_create(&ghost_thread_id, NULL, ghost_thread, &house.ghost);

  for (int i = 0; i < house.hunter_count; i++) {
    pthread_create(&hunter_threads[i], NULL, hunter_thread, &house.hunters[i]);
  }
  pthread_join(ghost_thread_id, NULL);
  for (int i = 0; i < house.hunter_count; i++) {
    pthread_join(hunter_threads[i], NULL);
  }
  free(hunter_threads);
  
  printf("\nGame Complete\n\n");


  printf("Hunter Result:\n");
  for (int i = 0; i < house.hunter_count; i++) {
    struct Hunter* hunter = &house.hunters[i];
    printf("  %s (ID: %d): %s\n", 
	   hunter->name, 
	   hunter->id, 
	   exit_reason_to_string(hunter->exit_reason));
  }
    


  printf("\nEvidence Collected: ");
  const enum EvidenceType* all_evidence = NULL;
  int count = get_all_evidence_types(&all_evidence);
  bool found_any = false;
  
  
  for (int i = 0; i < count; i++) {
    if (evidence_has(house.caseFile.collected, all_evidence[i])) {
      if (found_any) printf(", ");
      printf("%s", evidence_to_string(all_evidence[i]));
      found_any = true;
    }
  }
  if (!found_any) printf("None");
  printf("\n");
    
  printf("\nReal Ghost: %s\n", ghost_to_string(house.ghost.type));
    
  printf("Evidence Suggests: ");
  if (evidence_is_valid_ghost(house.caseFile.collected)) {
    const enum GhostType* ghost_types = NULL;
    int ghost_count = get_all_ghost_types(&ghost_types);
        
    for (int i = 0; i < ghost_count; i++) {
      if (house.caseFile.collected == (EvidenceByte)ghost_types[i]) {
	printf("%s\n", ghost_to_string(ghost_types[i]));
	break;
      }
    }
  } else {
    printf("(not enough or invalid evidence)\n");
  }
    
  
  printf("\nClean up\n");
  house_cleanup(&house);
    
  printf("Game ended successfully!\n");  
 
  return 0;
}