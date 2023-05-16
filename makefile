CC = gcc

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS   := -Wall -Werror -Wextra -Wno-unused-variable
LDLIBS   := -pthread

all:
	make proxy
	make client
	make server
	make poly

proxy: $(filter-out $(OBJ_DIR)/client.o $(OBJ_DIR)/server.o,$(OBJ)) 	 
	mkdir -p bin
	$(CC) $(filter-out $(OBJ_DIR)/client.o $(OBJ_DIR)/server.o,$^) $(LDLIBS) -o $(BIN_DIR)/$@

client: $(filter-out $(OBJ_DIR)/server.o $(OBJ_DIR)/proxy.o,$(OBJ)) 	 
	mkdir -p bin
	$(CC) $(filter-out $(OBJ_DIR)/server.o $(OBJ_DIR)/proxy.o,$^) $(LDLIBS) -o $(BIN_DIR)/$@

server: $(filter-out $(OBJ_DIR)/client.o $(OBJ_DIR)/proxy.o,$(OBJ)) 	 
	mkdir -p bin
	$(CC) $(filter-out $(OBJ_DIR)/client.o $(OBJ_DIR)/proxy.o,$^) $(LDLIBS) -o $(BIN_DIR)/$@

poly:
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC_DIR)/codepoly.c -o $(BIN_DIR)/codepoly -D MAIN
	
#creation .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
