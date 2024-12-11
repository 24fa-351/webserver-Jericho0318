webserver: webserver.c request.c
	gcc -o webserver webserver.c request.c -lpthread -lm

clean:
	rm webserver
	rm request