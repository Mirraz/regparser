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

$(EXECUTABLE): $(BUILD_DIR)/main.o $(BUILD_DIR)/regfile.o $(BUILD_DIR)/security_descriptor.o $(BUILD_DIR)/codepages.o
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/common.h $(SRC_DIR)/regfile.h $(SRC_DIR)/regfile_declare.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/regfile.o: $(SRC_DIR)/regfile.c $(SRC_DIR)/common.h $(SRC_DIR)/codepages.h $(SRC_DIR)/security_descriptor.h $(SRC_DIR)/regfile_declare.h $(SRC_DIR)/regfile.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/security_descriptor.o: $(SRC_DIR)/security_descriptor.c $(SRC_DIR)/common.h $(SRC_DIR)/security_descriptor_declare.h $(SRC_DIR)/security_descriptor.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/codepages.o: $(SRC_DIR)/codepages.c $(SRC_DIR)/codepages.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)



blocks: $(BUILD_DIR) blocks.out

blocks.out: $(BUILD_DIR)/blocks.o $(BUILD_DIR)/regfile.o $(BUILD_DIR)/security_descriptor.o $(BUILD_DIR)/codepages.o
	$(LD) -o blocks $^ $(LDFLAGS)

$(BUILD_DIR)/blocks.o: $(SRC_DIR)/blocks.c $(SRC_DIR)/common.h $(SRC_DIR)/regfile.h $(SRC_DIR)/regfile_declare.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

