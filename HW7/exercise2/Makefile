CC = gcc
CFLAGS = -Wall -pthread

SERVER = tcp_poll_server.o echo.o read_accounts.o
CLIENT = tcp_poll_client.o echo.o read_accounts.o

SERVER_NAME = server
CLIENT_NAME = client

all: $(SERVER_NAME) $(CLIENT_NAME)
	@echo "*** Build complete! ***"


# Exercise 1
$(SERVER_NAME): $(SERVER)
	$(CC) $(CFLAGS) $(SERVER) -o $(SERVER_NAME)

$(CLIENT_NAME): $(CLIENT)
	$(CC) $(CFLAGS) $(CLIENT) -o $(CLIENT_NAME)


clean:
	rm -f $(SERVER) $(SERVER_NAME) $(CLIENT) $(CLIENT_NAME)