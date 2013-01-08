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

$(EXECUTABLE): $(BUILD_DIR)/main.o $(BUILD_DIR)/widgets.o $(BUILD_DIR)/regfile.o $(BUILD_DIR)/rbtree.o $(BUILD_DIR)/string_type.o
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/widgets.h $(SRC_DIR)/regfile.h $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/widgets.o: $(SRC_DIR)/widgets.c $(SRC_DIR)/widgets.h $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/regfile.o: $(SRC_DIR)/regfile.c $(SRC_DIR)/regfile.h $(SRC_DIR)/regfile_declare.h $(SRC_DIR)/parse_common.h $(SRC_DIR)/rbtree.h $(SRC_DIR)/sglib.h $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/rbtree.o: $(SRC_DIR)/rbtree.c $(SRC_DIR)/rbtree.h $(SRC_DIR)/sglib.h $(SRC_DIR)/regfile.h $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/string_type.o: $(SRC_DIR)/string_type.c $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)


regfile_test_: $(BUILD_DIR) regfile_test

regfile_test: $(BUILD_DIR)/regfile_test.o $(BUILD_DIR)/regfile.o $(BUILD_DIR)/rbtree.o $(BUILD_DIR)/string_type.o
	$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/regfile_test.o: $(SRC_DIR)/regfile_test.c $(SRC_DIR)/regfile.h $(SRC_DIR)/string_type.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)
