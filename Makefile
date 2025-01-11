all: escalona procs/teste5 procs/teste10 procs/teste20 procs/teste30

escalona: scheduler.c main.c
	gcc -o escalona scheduler.c main.c

procs/teste5: teste5.c
	gcc -o procs/teste5 procs/teste5.c

procs/teste10: teste10.c
	gcc -o procs/teste10 procs/teste10.c

procs/teste20: teste20.c
	gcc -o procs/teste20 procs/teste20.c

procs/teste30: teste30.c
	gcc -o procs/teste30 procs/teste30.c

clean:
	rm -f escalona procs/teste5 procs/teste10 procs/teste20 procs/teste30