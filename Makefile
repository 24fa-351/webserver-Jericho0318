webserver: webserver.c http_message.c
	gcc -o webserver webserver.c http_message.c -lpthread -lm

clean:
	rm webserver
	rm http_message