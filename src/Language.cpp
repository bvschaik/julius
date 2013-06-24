#include "Language.h"

#include "Data/Language.h"

#include <stdio.h>

#define TEXT_HEADER_SIZE 28
#define TEXT_INDEX_ENTRIES 1000
#define TEXT_INDEX_SIZE 8000
#define TEXT_DATA_SIZE_MAX 200000

#define HELP_INDEX_SIZE 32024
#define HELP_DATA_SIZE_MAX 360000

static int fileLength(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return length;
}

int Language_load(const char *textfile, const char *helpfile)
{
	FILE *fp = fopen(textfile, "rb");
	if (!fp) {
		return 0;
	}
	int textDataLength = fileLength(fp) - (TEXT_HEADER_SIZE + TEXT_INDEX_SIZE);
	if (textDataLength > TEXT_DATA_SIZE_MAX) {
		fclose(fp);
		return 0;
	}
	fread(&Data_Language_Text.header, 1, TEXT_HEADER_SIZE, fp);
	fread(&Data_Language_Text.index, 1, TEXT_INDEX_SIZE, fp);
	fread(Data_Language_Text.data, 1, textDataLength, fp);
	fclose(fp);

	/*
	int fdHelp = _open(helpFile, O_BINARY);
	if (fdHelp == -1) {
		return 0;
	}
	int helpDataLength = _filelength(fdHelp) - HELP_INDEX_SIZE;
	if (helpDataLength > HELP_DATA_SIZE_MAX) {
		_close(fdHelp);
		return 0;
	}
	_read(fdHelp, helpIndex, HELP_INDEX_SIZE);
	_read(fdHelp, helpData, helpDataLength);
	_close(fdHelp);
	*/
	return 1;
}

const char *Language_getString(int group, int number)
{
	const char *str = &Data_Language_Text.data[Data_Language_Text.index[group].offset];
	char prev = 0;
	while (number > 0) {
		if (!*str && (prev >= ' ' || prev == 0)) { // nul byte
			--number;
		}
		prev = *str;
		++str;
	}
	while (*str < ' ') { // skip non-printables
		++str;
	}
	return str;
}
