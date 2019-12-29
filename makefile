all: Server Client
Server:
	g++ server_berkley.cpp -o server -pthread
Client:
	g++ slave_berkley.cpp -o client -pthread