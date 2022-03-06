CC=g++
CFLAGS= -std=c++20 -pedantic -Wall -Wextra -g

main: main.cpp
	$(CC) $(CFLAGS) -I curl/include/curl main.cpp client.cpp server.cpp graber.cpp -lcurl -o webgrab   
