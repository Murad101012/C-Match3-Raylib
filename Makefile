CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = ./libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
OBJ = main.o game_logic.o render.o input_gem_return.o game_config.o

# The name of your game
TARGET = match3

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
