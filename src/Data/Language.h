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

struct Data_Language_MessageEntry {
	short type;
	short messageType;
	short __unknown04;
	short x;
	short y;
	short widthBlocks;
	short heightBlocks;
	short picture1_graphicId;
	short picture1_x;
	short picture1_y;
	short picture2_graphicId;
	short picture2_x;
	short picture2_y;
	short titleX;
	short titleY;
	short subtitleX;
	short subtitleY;
	short __unknownX;
	short __unknownY;
	short videoX;
	short videoY;
	char __unknown22[14];
	int isUrgent;
	int videoLinkOffset;
	int __unknownOffset;
	int titleOffset;
	int subtitleOffset;
	int contentOffset;
};

extern struct Data_Language_Message {
	char header[24];
	struct Data_Language_MessageEntry index[400];
	char data [360000];
} Data_Language_Message;

#endif
