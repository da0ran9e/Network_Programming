CC = gcc
CFLAGS = -Wall

MSG_REQUIRED = msg/edit_file.o msg/msg_format.o msg/tcp_file.o msg/debug.o

SERVER = $(MSG_REQUIRED) tcp_select_server.o echo.o
CLIENT = $(MSG_REQUIRED) tcp_select_client.o echo.o

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