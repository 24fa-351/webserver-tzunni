webserver:	main.c
	gcc -o webserver main.c function.c -lpthread

clean:
	rm webserver
