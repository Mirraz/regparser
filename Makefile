CC=gcc
LD=gcc
WARNINGS=-Wall -Wextra
COPTIM=-march=native -mmmx -msse -msse2 -mssse3 -mcx16 -mfpmath=sse,387 -O2 -fomit-frame-pointer -pipe
DEFINES=
INCLUDES=
CFLAGS=$(WARNINGS) $(COPTIM) $(DEFINES) $(INCLUDES)
LDOPTIM=-Wl,-O1 -Wl,--as-needed
LIBFILES=
LDFLAGS=$(WARNINGS) $(LDOPTIM) $(LIBFILES)
SRC_DIR=src
BUILD_DIR=build
EXECUTABLE=regparser


all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(EXECUTABLE): $(BUILD_DIR)/main.o $(BUILD_DIR)/structs.o $(BUILD_DIR)/codepages.o 
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/structs.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/structs.o: $(SRC_DIR)/structs.c $(SRC_DIR)/structs.h $(SRC_DIR)/codepages.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/codepages.o: $(SRC_DIR)/codepages.c $(SRC_DIR)/codepages.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)
