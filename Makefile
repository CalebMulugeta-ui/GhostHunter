TARGETS = main.o helpers.o house.o room.o ghost.o hunter.o evidence.o roomstack.o

all: ${TARGETS}
	gcc -pthread -Wextra -Wall -Werror -o finalProject ${TARGETS}

main.o: main.c defs.h helpers.h
	gcc -g -c main.c

helpers.o: helpers.c defs.h helpers.h
	gcc -pthread -g -c helpers.c

house.o: house.c defs.h helpers.h
	gcc -g -c house.c

room.o: room.c defs.h helpers.h
	gcc -g -c room.c

ghost.o: ghost.c defs.h helpers.h
	gcc -g -c ghost.c

hunter.o: hunter.c defs.h helpers.h
	gcc -g -c hunter.c

evidence.o: evidence.c defs.h helpers.h
	gcc -g -c evidence.c

roomstack.o: roomstack.c defs.h
	gcc -g -c roomstack.c

clean:
	rm -f ${TARGETS} finalProject *.csv