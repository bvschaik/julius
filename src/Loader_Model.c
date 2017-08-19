#include "Loader.h"

#include "Data/Model.h"

#include "String.h"

#include "core/io.h"

#include <string.h>

#define TMP_BUFFER_SIZE 100000

static char buffer[TMP_BUFFER_SIZE];

static int stringsEqual(const char *a, const char *b, int len)
{
	for (int i = 0; i < len; i++) {
		if (a[i] != b[i]) {
			return 0;
		}
	}
	return 1;
}

static int indexOfString(const char *haystack, const char *needle, int haystackLength)
{
	int needleLength = strlen(needle);
	for (int i = 0; i < haystackLength; i++) {
		if (haystack[i] == needle[0] && stringsEqual(&haystack[i], needle, needleLength)) {
			return i + 1;
		}
	}
	return 0;
}

static int indexOf(const char *haystack, char needle, int haystackLength)
{
	for (int i = 0; i < haystackLength; i++) {
		if (haystack[i] == needle) {
			return i + 1;
		}
	}
	return 0;
}

static const char *skipNonDigits(const char *str)
{
	int safeguard = 0;
	while (1) {
		if (++safeguard >= 1000) {
			break;
		}
		if ((*str >= '0' && *str <= '9') || *str == '-') {
			break;
		}
		str++;
	}
	return str;
}


static const char *getValue(const char *ptr, int filesize, int *value) {
	ptr = skipNonDigits(ptr);
	*value = String_toInt(ptr);
	ptr += indexOf(ptr, ',', filesize);
	return ptr;
}


int Loader_Model_loadC3ModelTxt()
{
	memset(buffer, 0, sizeof(buffer));
	int filesize = io_read_file_into_buffer("c3_model.txt", buffer, TMP_BUFFER_SIZE);
	if (!filesize) {
		// file doesn't exist
		return 0;
	}

	int numLines = 0;
	int guard = 200;
	int curlyIndex;
	const char *ptr = &buffer[indexOfString(buffer, "ALL BUILDINGS", filesize)];
	do {
		guard--;
		curlyIndex = indexOf(ptr, '{', filesize);
		if (curlyIndex) {
			ptr += curlyIndex;
			numLines++;
		}
	} while (curlyIndex && guard > 0);

	if (numLines != 150) {
		// invalid model
		return 0;
	}

	int dummy;
	ptr = &buffer[indexOfString(buffer, "ALL BUILDINGS", filesize)];
	for (int i = 0; i < 130; i++) {
		ptr += indexOf(ptr, '{', filesize);

		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].cost);
		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].desirabilityValue);
		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].desirabilityStep);
		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].desirabilityStepSize);
		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].desirabilityRange);
		ptr = getValue(ptr, filesize, &Data_Model_Buildings[i].laborers);
		ptr = getValue(ptr, filesize, &dummy);
		ptr = getValue(ptr, filesize, &dummy);
	}

	ptr = &buffer[indexOfString(buffer, "ALL HOUSES", filesize)];
	
	for (int i = 0; i < 20; i++) {
		ptr += indexOf(ptr, '{', filesize);

		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].devolveDesirability);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].evolveDesirability);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].entertainment);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].water);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].religion);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].education);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].food);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].barber);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].bathhouse);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].health);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].foodTypes);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].pottery);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].oil);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].furniture);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].wine);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].__unused1);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].__unused2);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].prosperity);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].maxPeople);
		ptr = getValue(ptr, filesize, &Data_Model_Houses[i].taxMultiplier);
	}

	return 1;
}
