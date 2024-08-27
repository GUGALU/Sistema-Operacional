# get my-lib:
# https://github.com/ehmcruz/my-lib
MYLIB = ../my-lib

CC = gcc
CPP = g++
LD = g++
FLAGS = -std=c++23

ifdef CONFIG_TARGET_WINDOWS
	FLAGS += -DNCURSES_STATIC=1 -DCONFIG_TARGET_WINDOWS=1
endif

ifdef CONFIG_TARGET_LINUX
	FLAGS += -DCONFIG_TARGET_LINUX=1
endif

CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS) -I$(MYLIB)/include -Wall
LDFLAGS = -lncurses
BIN_NAME = arq-sim-so
RM = rm

# -fprofile-arcs -ftest-coverage

########################################################

SRC = $(wildcard *.cpp)

headerfiles = $(wildcard *.h)

OBJS = ${SRC:.cpp=.o}

########################################################

# implicit rules

%.o : %.c $(headerfiles)
	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.cpp $(headerfiles)
	$(CPP) -c $(CPPFLAGS) $< -o $@

########################################################

all: $(BIN_NAME)
	@echo program compiled!
	@echo yes!

$(BIN_NAME): $(OBJS)
	$(LD) -o $(BIN_NAME) $(OBJS) $(LDFLAGS)

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)

