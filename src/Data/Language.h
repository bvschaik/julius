#ifndef DATA_LANGUAGE_H
#define DATA_LANGUAGE_H

extern struct Data_Language_Text {
	char header[28];
	struct {
		int offset;
		int inUse;
	} index[1000];
	char data[200000];
} Data_Language_Text;

#endif
