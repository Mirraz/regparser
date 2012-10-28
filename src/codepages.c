#include <stdio.h>
#include <assert.h>
#include <iconv.h>

#include "codepages.h"

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
