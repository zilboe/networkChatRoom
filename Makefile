all:
	gcc client/client.c -o cli -lpthread
	gcc server/server.c -o ser -lpthread
clean:
	rm -rf cli ser
