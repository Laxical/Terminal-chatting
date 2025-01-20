compile:
	gcc tcp_server.c -o ./obj/tcp_server
	gcc tcp_client.c -o ./obj/tcp_client

clean:
	rm -rf ./obj/tcp_server
	rm -rf ./obj/tcp_client