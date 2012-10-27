#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <endian.h>
#include <iconv.h>

void fprintStringUnicode(FILE *fout, const unsigned char *str_words, unsigned int len) {
	#ifndef MINGW32
	char *pIn;
	size_t inLen;
	const int c_outBuf_size = 9;
	char outBuf[c_outBuf_size];
	outBuf[c_outBuf_size-1] = 0;
	char *pOut;
	size_t outLen;
	iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
	if (cd == (iconv_t)(-1)) return;
	unsigned int i;
	for (i=0; i<len; ++i) {
		pIn = (char *)&str_words[i*2];
		inLen = 2;
		outBuf[0] = outBuf[1] = outBuf[2] = outBuf[3] = 0;
		outBuf[4] = outBuf[5] = outBuf[6] = outBuf[7] = 0;
		pOut = &(outBuf[0]);
		outLen = c_outBuf_size;
		iconv(cd, &pIn, &inLen, &pOut, &outLen);
		fprintf(fout, "%s", outBuf);
	}
	iconv_close(cd);
	#else
	char sym_cp866;
	int i;
	for (i=0; i<len; ++i) {
		WideCharToMultiByte(866, 0, (wchar_t *)(&str_words[i*2]), 1, &sym_cp866, 1, NULL, NULL);
		fputc(sym_cp866, fout);
	}
	#endif
}

#define print(format, args...) do{fprintf(stdout, format, ##args);}while(0)
#define printl(format, args...) do{fprintf(stdout, format "\n", ##args);}while(0)

/* ********************************** */

typedef struct Base_ Base;

typedef struct {
	void (*bytes_order_to_h) (Base *);
	int (*load_by_current_offset) (Base *, FILE *);
	size_t size;
} BaseClass;

struct Base_ {
	BaseClass *c;
};

int Base_load_by_offset(Base *h, FILE *file, long offset) {
	int res = fseek(file, offset, SEEK_SET);
	assert(res == 0);
	return h->c->load_by_current_offset(h, file);
}

/* ********************************** */

#pragma pack(push,1)
typedef struct {
	uint8_t signature[4];
	uint32_t opened_transaction;
	uint32_t closed_transaction;
	uint64_t begin_transaction_number;
	uint32_t stuff1;
	uint32_t file_version;
	uint32_t subversion;
	uint32_t stuff2;
	uint32_t root_key_address;
	uint32_t data_area_size;
	uint32_t stuff3;
	uint8_t reg_file_path[460];
	uint32_t checksum;
	uint8_t padding[3584];
} __attribute__ ((__packed__)) HeaderStruct;
#pragma pack(pop)

typedef struct Header_ Header;

typedef struct {
	/* from Base */
	void (*bytes_order_to_h) (Header *);
	int (*load_by_current_offset) (Header *, FILE *);
	size_t size;
	/* specific */
} HeaderClass;

struct Header_ {
	HeaderClass *c;
	HeaderStruct d;
};

void Header_class_init();
void Header_bytes_order_to_h(Header *h);
int Header_load_by_current_offset(Header *h, FILE *file);

static HeaderClass header_class = {
	.bytes_order_to_h = &Header_bytes_order_to_h,
	.load_by_current_offset = &Header_load_by_current_offset,
	.size = sizeof(HeaderStruct)
};

#define Header_init_value {.c = &header_class}

/* ------------- */

void Header_class_init() {
	assert(sizeof(HeaderStruct) == 0x1000);
}

void Header_bytes_order_to_h(Header *h) {
	assert(h != NULL);
	h->d.opened_transaction = le32toh(h->d.opened_transaction);
	h->d.closed_transaction = le32toh(h->d.closed_transaction);
	h->d.begin_transaction_number = le64toh(h->d.begin_transaction_number);
	h->d.stuff1 = le32toh(h->d.stuff1);
	h->d.file_version = le32toh(h->d.file_version);
	h->d.subversion = le32toh(h->d.subversion);
	h->d.stuff2 = le32toh(h->d.stuff2);
	h->d.root_key_address = le32toh(h->d.root_key_address);
	h->d.data_area_size = le32toh(h->d.data_area_size);
	h->d.stuff3 = le32toh(h->d.stuff3);
	h->d.checksum = le32toh(h->d.checksum);
}

int Header_load_by_current_offset(Header *h, FILE *file) {
	assert(h != NULL);
	assert(file != NULL);
	size_t red = fread(&(h->d), 1, sizeof(h->d), file);
	assert(red == sizeof(h->d));
	Header_bytes_order_to_h(h);
	return 0;
}

void Header_print(Header *h) {
	assert(h != NULL);

	char *sig = (char *)&(h->d.signature);
	printl("signature = %c%c%c%c", sig[0], sig[1], sig[2], sig[3]);

	printl("opened_transaction = %08X", h->d.opened_transaction);
	printl("closed_transaction = %08X", h->d.closed_transaction);
	printl("begin_transaction_number = %016X", (unsigned int)h->d.begin_transaction_number);
	printl("stuff1 = %08X", h->d.stuff1);
	printl("file_version = %08X", h->d.file_version);
	printl("subversion = %08X", h->d.subversion);
	printl("stuff2 = %08X", h->d.stuff2);
	printl("root_key_address = %08X", h->d.root_key_address);
	printl("data_area_size = %08X", h->d.data_area_size);
	printl("stuff3 = %08X", h->d.stuff3);

	unsigned int i;

	printl("reg_file_path = ");
	fprintStringUnicode(stdout, h->d.reg_file_path, 460/2);
	printl("");

	printl("checksum = %08X", h->d.checksum);

	for (i=0; i<3584; ++i)
		if (h->d.padding[i] != 0)
			printl("padding[%d] = %02X", i, h->d.padding[i]);
}


/* ********************************** */

void Classes_init() {
	Header_class_init();
}

int main (int argc, char *argv[]) {
	Classes_init();

	static Header h = Header_init_value;

	Base *p = (Base *)&h;

	assert(argc == 2);
	FILE *regfile = fopen(argv[1], "r");
	Base_load_by_offset(p, regfile, 0);

	Header_print((Header *)p);

	return 0;
}

