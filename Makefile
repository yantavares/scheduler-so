all: escalona process/teste5 process/teste10 process/teste20 process/teste30

escalona: scheduler.c main.c
	gcc -o escalona scheduler.c main.c

process/teste5: process/teste5.c
	gcc -o process/teste5 process/teste5.c

process/teste10: process/teste10.c
	gcc -o process/teste10 process/teste10.c

process/teste20: process/teste20.c
	gcc -o process/teste20 process/teste20.c

process/teste30: process/teste30.c
	gcc -o process/teste30 process/teste30.c

clean:
	rm -f escalona process/teste5 process/teste10 process/teste20 process/teste30