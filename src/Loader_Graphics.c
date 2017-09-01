#include "Data/Graphics.h"

#include <stdlib.h>
#include <string.h>

#include "Graphics.h"
#include "Graphics_private.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#include "core/buffer.h"
#include "core/debug.h"
#include "core/file.h"
#include "core/io.h"

// NOTE done x4/x2, revert to more conservative size after testing
#define MAIN_SIZE 48400000
#define ENEMY_SIZE 9800000
#define SCRATCH_SIZE 12100000

static const char mainGraphicsSg2[][32] = {
	"c3.sg2",
	"c3_north.sg2",
	"c3_south.sg2"
};
static const char mainGraphics555[][32] = {
	"c3.555",
	"c3_north.555",
	"c3_south.555"
};
static const char enemyGraphicsSg2[][32] = {
	"goths.sg2",
	"Etruscan.sg2",
	"Etruscan.sg2",
	"carthage.sg2",
	"Greek.sg2",
	"Greek.sg2",
	"egyptians.sg2",
	"Persians.sg2",
	"Phoenician.sg2",
	"celts.sg2",
	"celts.sg2",
	"celts.sg2",
	"Gaul.sg2",
	"Gaul.sg2",
	"goths.sg2",
	"goths.sg2",
	"goths.sg2",
	"Phoenician.sg2",
	"North African.sg2",
	"Phoenician.sg2",
};
static const char enemyGraphics555[][32] = {
	"goths.555",
	"Etruscan.555",
	"Etruscan.555",
	"carthage.555",
	"Greek.555",
	"Greek.555",
	"egyptians.555",
	"Persians.555",
	"Phoenician.555",
	"celts.555",
	"celts.555",
	"celts.555",
	"Gaul.555",
	"Gaul.555",
	"goths.555",
	"goths.555",
	"goths.555",
	"Phoenician.555",
	"North African.555",
	"Phoenician.555",
};

static int currentClimate = -1;

static void prepareMainGraphics(buffer *buf);
static void prepareEnemyGraphics(buffer *buf);

static char *imagesScratchSpace;

int Loader_Graphics_initGraphics()
{
	Data_Graphics_PixelData.enemy = (Color*) malloc(ENEMY_SIZE);
	Data_Graphics_PixelData.main = (Color*) malloc(MAIN_SIZE);
	imagesScratchSpace = (char*) malloc(SCRATCH_SIZE);
	if (!Data_Graphics_PixelData.main || !Data_Graphics_PixelData.enemy || !imagesScratchSpace) {
		return 0;
	}
	Graphics_initialize();
	return 1;
}

int Loader_Graphics_loadMainGraphics(int climate)
{
	if (climate == currentClimate) {
		return 1;
	}

	const char *filename555 = mainGraphics555[climate];
	const char *filenameSg2 = mainGraphicsSg2[climate];
	
	if (!io_read_file_into_buffer(filenameSg2, &Data_Graphics_Main, sizeof(Data_Graphics_Main))) {
		return 0;
	}
	int size = io_read_file_into_buffer(filename555, imagesScratchSpace, SCRATCH_SIZE);
	if (!size) {
		return 0;
	}
	buffer buf;
    buffer_init(&buf, imagesScratchSpace, size);

	prepareMainGraphics(&buf);
	currentClimate = climate;
	return 1;
}

int Loader_Graphics_loadEnemyGraphics(int enemyId)
{
	const char *filename555 = enemyGraphics555[enemyId];
	const char *filenameSg2 = enemyGraphicsSg2[enemyId];

	if (!io_read_file_part_into_buffer(filenameSg2, &Data_Graphics_Enemy, 51264, 20680)) {
		return 0;
	}
	int size = io_read_file_into_buffer(filename555, imagesScratchSpace, SCRATCH_SIZE);
	if (!size) {
		return 0;
	}
	buffer buf;
    buffer_init(&buf, imagesScratchSpace, size);

	prepareEnemyGraphics(&buf);
	return 1;
}

static int convertUncompressed(buffer *buf, int buf_length, Color *dst)
{
    for (int i = 0; i < buf_length; i += 2) {
        *dst = ColorLookup[buffer_read_u16(buf)];
        dst++;
    }
    return buf_length / 2;
}

static int convertCompressed(buffer *buf, int buf_length, Color *dst)
{
    int dst_length = 0;
    while (buf_length > 0) {
        int control = buffer_read_u8(buf);
        if (control == 255) {
            // next byte = transparent pixels to skip
            *(dst++) = 255;
            *(dst++) = buffer_read_u8(buf);
            dst_length += 2;
            buf_length -= 2;
        } else {
            // control = number of concrete pixels
            *(dst++) = control;
            for (int i = 0; i < control; i++) {
                *(dst++) = ColorLookup[buffer_read_u16(buf)];
            }
            dst_length += control + 1;
            buf_length -= control * 2 + 1;
        }
    }
    return dst_length;
}

static void convertGraphics(struct Data_Graphics_Index *indexList, int size, buffer *buf, Color *dst)
{
    Color *orig_dst = dst;
    for (int img = 0; img < size; img++) {
        struct Data_Graphics_Index *index = &indexList[img];
        if (index->isExternal) {
            continue;
        }
        buffer_reset(buf);
        buffer_skip(buf, index->offset);
        
        int img_offset = dst - orig_dst;
        if (index->isFullyCompressed) {
            dst += convertCompressed(buf, index->dataLength, dst);
        } else if (index->hasCompressedPart) {
            dst += convertUncompressed(buf, index->uncompressedLength, dst);
            dst += convertCompressed(buf, index->dataLength - index->uncompressedLength, dst);
        } else {
            dst += convertUncompressed(buf, index->dataLength, dst);
        }
        index->offset = img_offset;
        index->uncompressedLength /= 2;
    }
}

const Color *Loader_Graphics_loadExternalImagePixelData(int graphicId)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	char filename[200] = "555/";
	strcpy(&filename[4], Data_Graphics_Main.bitmaps[(int)index->bitmapId]);
	file_change_extension(filename, "555");
    int size = io_read_file_part_into_buffer(
            &filename[4], imagesScratchSpace,
            index->dataLength, index->offset - 1);
	if (!size) {
		// try in 555 dir
        size = io_read_file_part_into_buffer(
                filename, imagesScratchSpace,
                index->dataLength, index->offset - 1);
		if (!size) {
            debug_log("Unable to read external file", filename, 0);
			return 0;
		}
	}
    buffer buf;
    buffer_init(&buf, imagesScratchSpace, size);
    Color *dst = (Color*) &imagesScratchSpace[4000000];
    if (index->isFullyCompressed) {
        convertCompressed(&buf, index->dataLength, dst);
    } else if (index->hasCompressedPart) {
        convertUncompressed(&buf, index->uncompressedLength, dst);
        convertCompressed(&buf, index->dataLength - index->uncompressedLength, dst);
    } else {
        convertUncompressed(&buf, index->dataLength, dst);
    }
	return dst;
}

static void prepareMainGraphics(buffer *buf)
{
	int offset = 4;
	for (int i = 1; i < 10000; i++) {
		struct Data_Graphics_Index *index = &Data_Graphics_Main.index[i];
		if (index->isExternal) {
			if (!index->offset) {
				index->offset = 1;
			}
			continue;
		}
		// internal graphic
		index->offset = offset;
		offset += index->dataLength;
	}
	convertGraphics(Data_Graphics_Main.index, 10000, buf, Data_Graphics_PixelData.main);
}

static void prepareEnemyGraphics(buffer *buf)
{
	int offset = 4;
	for (int i = 1; i <= 800; i++) {
		struct Data_Graphics_Index *index = &Data_Graphics_Enemy.index[i];
		if (index->isExternal) {
			if (!index->offset) {
				index->offset = 1;
			}
			continue;
		}
		// internal graphic
		index->offset = offset;
		offset += index->dataLength;
	}
	convertGraphics(Data_Graphics_Enemy.index, 801, buf, Data_Graphics_PixelData.enemy);
}
