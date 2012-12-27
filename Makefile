CC=gcc
LD=gcc
WARNINGS=-Wall -Wextra
COPTIM=-march=native -mmmx -msse -msse2 -mssse3 -mcx16 -mfpmath=sse,387 -O2 -fomit-frame-pointer -pipe
DEFINES=
INCLUDES=
CFLAGS=$(WARNINGS) $(COPTIM) $(DEFINES) $(INCLUDES)
LDOPTIM=-Wl,-O1 -Wl,--as-needed
LIBFILES=-lncursesw
LDFLAGS=$(WARNINGS) $(LDOPTIM) $(LIBFILES)
SRC_DIR=src
BUILD_DIR=build
EXECUTABLE=regparser


all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(EXECUTABLE): $(BUILD_DIR)/main.o
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)
