all: Server Client
Server:
	g++ server_berkley.cpp -o server -pthread
Client:
	g++ slave_berkley.cpp -o client -pthread
clean:
	rm server client
all:
	g++ -std=c++11 server_berkley.cpp -o server -pthread
	g++ -std=c++11 slave_berkley.cpp -o client -pthread
