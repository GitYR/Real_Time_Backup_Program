RESULT_OBJECTS = main.o ssu_backup.o ssu_support.o linkedlist.o backuplist.o

ssu_backup : $(RESULT_OBJECTS)
	gcc -o ssu_backup main.o ssu_backup.o ssu_support.o linkedlist.o backuplist.o -g -lpthread
	make clean

main.o : ssu_backup.h main.c
	gcc -c -o main.o main.c -lpthread

ssu_backup.o : ssu_backup.h ssu_backup.c
	gcc -c -o ssu_backup.o ssu_backup.c -lpthread

ssu_support.o : ssu_backup.h ssu_support.c
	gcc -c -o ssu_support.o ssu_support.c -lpthread

linkedlist.o : ssu_backup.h linkedlist.c
	gcc -c -o linkedlist.o linkedlist.c 

backuplist.o : ssu_backup.h backuplist.c
	gcc -c -o backuplist.o backuplist.c

clean :
	rm $(RESULT_OBJECTS)
