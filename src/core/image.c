#include "image.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 20680
#define ENTRY_SIZE 64

#define MAIN_ENTRIES 10000
#define ENEMY_ENTRIES 801
#define CYRILLIC_FONT_ENTRIES 2000
#define TRAD_CHINESE_FONT_ENTRIES (3 * IMAGE_FONT_MULTIBYTE_CHINESE_MAX_CHARS)
#define KOREAN_FONT_ENTRIES (3 * IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS)

#define MAIN_INDEX_SIZE 660680
#define ENEMY_INDEX_OFFSET HEADER_SIZE
#define ENEMY_INDEX_SIZE ENTRY_SIZE * ENEMY_ENTRIES
#define CYRILLIC_FONT_INDEX_OFFSET HEADER_SIZE
#define CYRILLIC_FONT_INDEX_SIZE ENTRY_SIZE * CYRILLIC_FONT_ENTRIES

#define MAIN_DATA_SIZE 30000000
#define EMPIRE_DATA_SIZE (2000*1000*4)
#define ENEMY_DATA_SIZE 2400000
#define CYRILLIC_FONT_DATA_SIZE 1500000
#define TRAD_CHINESE_FONT_DATA_SIZE 7000000
#define KOREAN_FONT_DATA_SIZE 7500000
#define SCRATCH_DATA_SIZE 12100000

#define CYRILLIC_FONT_BASE_OFFSET 201

#define NAME_SIZE 32

enum {
    NO_EXTRA_FONT = 0,
    FULL_CHARSET_IN_FONT = 1,
    MULTIBYTE_IN_FONT = 2
};

static const char MAIN_GRAPHICS_SG2[][NAME_SIZE] = {
    "c3.sg2",
    "c3_north.sg2",
    "c3_south.sg2"
};
static const char MAIN_GRAPHICS_555[][NAME_SIZE] = {
    "c3.555",
    "c3_north.555",
    "c3_south.555"
};
static const char EDITOR_GRAPHICS_SG2[][NAME_SIZE] = {
    "c3map.sg2",
    "c3map_north.sg2",
    "c3map_south.sg2"
};
static const char EDITOR_GRAPHICS_555[][NAME_SIZE] = {
    "c3map.555",
    "c3map_north.555",
    "c3map_south.555"
};
static const char EMPIRE_555[NAME_SIZE] = "The_empire.555";

static const char CYRILLIC_FONTS_SG2[NAME_SIZE] = "C3_fonts.sg2";
static const char CYRILLIC_FONTS_555[NAME_SIZE] = "C3_fonts.555";
static const char TRAD_CHINESE_FONTS_555[NAME_SIZE] = "rome.555";
static const char KOREAN_FONTS_555[NAME_SIZE] = "korean.555";

static const char ENEMY_GRAPHICS_SG2[][NAME_SIZE] = {
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
static const char ENEMY_GRAPHICS_555[][NAME_SIZE] = {
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

static const image DUMMY_IMAGE;

static struct {
    int current_climate;
    int is_editor;
    int fonts_enabled;
    int font_base_offset;

    uint16_t group_image_ids[300];
    char bitmaps[100][200];
    image main[MAIN_ENTRIES];
    image enemy[ENEMY_ENTRIES];
    image *font;
    color_t *main_data;
    color_t *empire_data;
    color_t *enemy_data;
    color_t *font_data;
    uint8_t *tmp_data;
} data = {.current_climate = -1};

static const image roadblock_image = { 58,30,0,0,0,0,0,{30,0,0,0,10000,0,1800,900} };
static color_t roadblock_data[900] = { 0xa4180f, 0xa71f15
            , 0xa3150c, 0xa4170d, 0xaf4936, 0xd5ddb5, 0xced3a4, 0xc05c44
            , 0xa3150c, 0xaf3324, 0xb84837, 0xe1e3b1, 0xffecd0, 0xf5f2f5, 0xfff0d6, 0xf6f1fa, 0xe5adb0, 0xae2d24
            , 0xb2392c, 0xb43d2e, 0xde907a, 0xfff6b7, 0xffdac6, 0xfeefe2, 0xfdf5df, 0xfffbdb, 0xffefd8, 0xffe6c7, 0xe5d8b4, 0xd08169, 0xad2822, 0xa71d15
            , 0xa82217, 0xa71f15, 0xc96b5c, 0xf8eadf, 0xfff2e1, 0xfdefe0, 0xf9eedd, 0xf7d8c7, 0xedac97, 0xecbaa5, 0xfae8d4, 0xfff4d8, 0xffe8d0, 0xffebc6, 0xf2eef2, 0xdfbfc0, 0xc05849, 0xa82217
            , 0xaa261b, 0xaf3228, 0xcf7f70, 0xf1cfaa, 0xffe7c9, 0xfff0dd, 0xfeefdf, 0xf4c1ad, 0xdd7465, 0xb33129, 0xa1100c, 0xaa1c16, 0xc14334, 0xdc8268, 0xf3bca6, 0xfde3ca, 0xf9f8db, 0xfff8e2, 0xffecd1, 0xfcead3, 0xc99272, 0xac2d1e
            , 0xad2d22, 0xad2d20, 0xcd735e, 0xf4dac1, 0xffeedf, 0xfff5e1, 0xfbefe1, 0xedc1b8, 0xc64f50, 0xab2020, 0xad1e1c, 0xc1322c, 0xcb332f, 0xc22824, 0xaf1916, 0xa91411, 0xb3231c, 0xd3634e, 0xf6b898, 0xfee4ce, 0xffecdd, 0xfff6b9, 0xffe4c1, 0xfeeabf, 0xdea281, 0xb43e2d
            , 0xaa271c, 0xa9271d, 0xd68c79, 0xfde8c7, 0xf6dab7, 0xffeacd, 0xfdead8, 0xf2bea7, 0xd35c57, 0xad1f1f, 0xa61512, 0xbc2a23, 0xcd3229, 0xd7382e, 0xda3a31, 0xd6372d, 0xcf3128, 0xc52a22, 0xb31b15, 0xa5130e, 0xae251c, 0xd8755d, 0xeeac96, 0xfde9d9, 0xffecd3, 0xffe5d2, 0xffeac3, 0xffe6c6, 0xe0a27f, 0xaf3023
            , 0xa61b12, 0xb23629, 0xdd927c, 0xffeab9, 0xf6e1b7, 0xfff4e4, 0xfdf2e5, 0xf4c6b3, 0xe27d71, 0xb9332f, 0xa20c08, 0xa7100a, 0xb61b0f, 0xc02112, 0xc32413, 0xc32413, 0xc32413, 0xc32413, 0xc32413, 0xc22313, 0xbe2011, 0xb61b0e, 0xa70f09, 0x9c0806, 0xac1b12, 0xe18a69, 0xf7c0a3, 0xfeebd8, 0xffeee5, 0xffefd2, 0xf6dbb5, 0xffebbd, 0xdc9b7b, 0xab281c
            , 0xaa241b, 0xbf5543, 0xf1c19b, 0xfce0b4, 0xffedd6, 0xfff1df, 0xf9e5db, 0xedb6ab, 0xe0837c, 0xc34946, 0xa11c18, 0x9b1914, 0xc26456, 0xedc1a5, 0xe8dcd7, 0xe6d2c1, 0xe6cec2, 0xe7d0c6, 0xf0e4b5, 0xfed3b3, 0xfedeb2, 0xf6e7b9, 0xffecbb, 0xfde9ce, 0xefbdb2, 0xd26f67, 0xaf312d, 0xab241d, 0xca5945, 0xe79577, 0xf5ccb4, 0xfdf1d9, 0xfff2c5, 0xffecda, 0xffe5c6, 0xfee3c4, 0xd79083, 0xad2d25
            , 0xb03328, 0xbd4c3e, 0xdca28f, 0xffdecd, 0xffe8dd, 0xfff0e1, 0xfbe1d0, 0xeba997, 0xcf605c, 0xae2928, 0xa01210, 0x9b1513, 0x840f06, 0x6d1303, 0xbdae77, 0xfef6dc, 0xfffaee, 0xfff6e7, 0xfff3e7, 0xfff5ec, 0xfff9ee, 0xfffbf2, 0xfff9ee, 0xfffbf0, 0xfff6e7, 0xfff7ea, 0xfff9f0, 0xfbece1, 0xe7ab95, 0xc2554f, 0xa7201c, 0xaf2921, 0xc44d3a, 0xe28a6d, 0xf6c8b2, 0xfeeed6, 0xffe9d2, 0xe6d0b6, 0xffdcc3, 0xfbd6d0, 0xd1766c, 0xae2c23
            , 0xb03429, 0xb1342b, 0xe5ba8e, 0xf4e1b5, 0xf3e4b8, 0xfff3de, 0xfbe4d4, 0xefa795, 0xd66660, 0xb12725, 0xa10d0a, 0xb33837, 0x9e4841, 0x6e2e18, 0x661504, 0x651f08, 0xd2b48d, 0xfebba5, 0xf9c3b6, 0xf6c2b6, 0xf6cbc2, 0xf6c1b5, 0xf5c0b3, 0xf4bcaf, 0xf4bbaf, 0xf4bcb0, 0xf2afa1, 0xf2b3a2, 0xf8c3b6, 0xfcd6c0, 0xfdd5c8, 0xf5c8bd, 0xe5a18c, 0xc85e55, 0xaa2621, 0xa51914, 0xc0402e, 0xe68f70, 0xf7cebb, 0xfef1e7, 0xffedd4, 0xffe4be, 0xfff0e6, 0xfad8ba, 0xc9725e, 0xa82016
            , 0xad2b20, 0xb64532, 0xdda282, 0xffcdde, 0xffdbc5, 0xfff6ec, 0xf8e1d3, 0xeaa590, 0xd86a63, 0xbb3333, 0xad1d19, 0xb1281b, 0xb51b0e, 0xf0cebf, 0xecdfd2, 0xd8c6a8, 0xcdbc9c, 0xdfc9b4, 0xfda087, 0xe53c29, 0xc82817, 0xbc2010, 0xbf2212, 0xbd2010, 0xbe1f0f, 0xbb1f0f, 0xbb1f0f, 0xbd1f0f, 0xbb1e0e, 0xbb1f0f, 0xc72716, 0xdc765f, 0xf5bca7, 0xfed8bb, 0xfff0df, 0xf7e0d5, 0xe7a399, 0xbf4e45, 0xa30f0a, 0xac221a, 0xca4f3a, 0xe89980, 0xf9ded0, 0xfff9ea, 0xffead5, 0xf0d9b5, 0xe6d4b6, 0xe5cab1, 0xc25a4a, 0xa51a10
            , 0xa4180e, 0xb84e3a, 0xeac49c, 0xfdf3b1, 0xf6f1b6, 0xfeeddf, 0xfaecde, 0xf2bdab, 0xcc5b51, 0xb02824, 0xa51411, 0xb21c19, 0xca2f2b, 0xd73d29, 0xc22414, 0xffedd4, 0xfff2ea, 0xffede0, 0xfff3e5, 0xffded1, 0xf96f5a, 0xc82819, 0xcb2817, 0xbb1e0e, 0xc22313, 0xc72f1d, 0xc93221, 0xce3c2a, 0xd7503b, 0xd85640, 0xd44d39, 0xc94333, 0xa92e21, 0x88281d, 0xc68172, 0xfde3c8, 0xfff5e7, 0xfff7ea, 0xfef6dc, 0xe9b09e, 0xb91d11, 0xc1392a, 0xaa1712, 0xb2241b, 0xd15e45, 0xe9a791, 0xfbe6d5, 0xfffce2, 0xffe8dc, 0xf0dab5, 0xf2d8b7, 0xf4d4cb, 0xbd5750, 0xaa271c
            , 0xae3227, 0xa72116, 0xd4966f, 0xe5e1b5, 0xfff9c1, 0xffeacd, 0xfff3e0, 0xfad8c1, 0xdc8276, 0xc6564f, 0xa31614, 0xbc382b, 0xd73d2f, 0xe1433e, 0xea4a45, 0xe34335, 0xc32514, 0xfff4e2, 0xfff7ee, 0xfff1e3, 0xfff5ea, 0xffe2d8, 0xef614d, 0xbd200f, 0xc32414, 0xbe1f0f, 0xe5432f, 0xf2a282, 0xf4c4ad, 0xf6cab3, 0xf9decd, 0xf9d6c3, 0xefa9a0, 0xa64841, 0x680e04, 0x670f05, 0xa05248, 0xfbddc5, 0xfff4e0, 0xfff5ec, 0xfff1e3, 0xf1c1b4, 0xc22615, 0xe94f37, 0xd53833, 0xbb241d, 0xa4140f, 0xae2921, 0xd67866, 0xf9d5c1, 0xfff1e1, 0xfff5de, 0xfff2d4, 0xffebd9, 0xf6e5e3, 0xb58d71, 0xa63429, 0xa51b12
            , 0xa52015, 0xa4271a, 0x96674e, 0xd1be8c, 0xfcefc6, 0xfff7e1, 0xffede1, 0xf8cdb1, 0xcf756a, 0xaa2722, 0xaf332e, 0xd05048, 0xe94d40, 0xef4e49, 0xef4e49, 0xde3e30, 0xc32413, 0xfff4e4, 0xfff3e7, 0xfff6e7, 0xfff5ee, 0xffddce, 0xed5e48, 0xbd1f0f, 0xbd1f0e, 0xbb1e0e, 0xf55a43, 0xffdac2, 0xfef2e2, 0xfdf1de, 0xfeefe0, 0xfbdcd1, 0xea9f97, 0xa14b3e, 0x711910, 0x6c150b, 0xa2594c, 0xfadbc1, 0xfff5e7, 0xfff7f0, 0xfff1e1, 0xf0c3b8, 0xc32615, 0xf05544, 0xec4b46, 0xdd443a, 0xc7483f, 0xa8201c, 0xa31613, 0xeea494, 0xffdece, 0xfff1e1, 0xfffbe7, 0xd6c29f, 0xa38472, 0xa96658, 0xa3160d, 0xa61d14
            , 0xa52217, 0xa5352d, 0xa97766, 0xc6ad90, 0xfcf4dd, 0xfffbeb, 0xfbdfcc, 0xe9a48f, 0xb53f36, 0xa8201b, 0xc93d2e, 0xdf433a, 0xea4a45, 0xe2432c, 0xc32413, 0xfff6e2, 0xfff6e7, 0xfff8e5, 0xfff2e6, 0xffdecb, 0xe54f3b, 0xc02111, 0xb61e0e, 0xc72c1b, 0xdd7663, 0xc48b7a, 0xc08074, 0xc17b73, 0xc4877c, 0xc57473, 0xbc6d6b, 0x934138, 0x711910, 0x6c150b, 0xa55e4f, 0xfadcbe, 0xfff5e5, 0xfff7ea, 0xfff1d9, 0xf1bfae, 0xc22514, 0xe75b40, 0xd33f36, 0xb92f29, 0xaa221e, 0xb6423a, 0xeca896, 0xffdcc3, 0xfff4e3, 0xead5b1, 0x9e8673, 0x9d4338, 0xa51f16, 0xa3150c
            , 0xa41910, 0xa64b44, 0xaa8984, 0xd2bfb9, 0xfde3c0, 0xfdf6e1, 0xf9d9c7, 0xde8c7b, 0xaf2d27, 0xb32a23, 0xcd3f2e, 0xdb4c26, 0xbf2414, 0xfcede3, 0xfffdf4, 0xfff7ee, 0xfff0e9, 0xffd3d1, 0xc22414, 0xc32b1a, 0x912719, 0x7c2015, 0x7e291f, 0x6c1309, 0x6b1107, 0x6b1208, 0x6c1309, 0x6c1309, 0x6e150c, 0x6e170d, 0x660f05, 0x6f1910, 0xb16d5b, 0xecd0b5, 0xd4c4b2, 0xdaccc0, 0xf3e0d2, 0xe19e94, 0xb41a0f, 0xbc3b2d, 0xa1100e, 0xbd473e, 0xeda894, 0xfcddc6, 0xfcf3e6, 0xe7d0bb, 0xb29b87, 0xa46959, 0xa41e14, 0xa4160d
            , 0xa62319, 0xa34138, 0xa7775e, 0xcab695, 0xf2eab2, 0xf8ead3, 0xfee0cb, 0xde8a76, 0xb13027, 0xae2820, 0xb42b22, 0xd9887e, 0xecbdb2, 0xfcece2, 0xffefe4, 0xfecdbb, 0xec624b, 0xd27160, 0x731e14, 0x762319, 0x86362c, 0x741d12, 0x721b10, 0x711a0f, 0x70180d, 0x6f170c, 0x6e160c, 0x6f190e, 0x7c2b1f, 0x954c3c, 0xc28b70, 0x742714, 0x60361c, 0x7d482d, 0xc1655c, 0xb53a39, 0xa51914, 0xc1524a, 0xf1ad9d, 0xfbd8c9, 0xfef2d9, 0xebd5c1, 0xb4aba9, 0xa88984, 0xa6493f, 0xa41d14
            , 0xa5251b, 0xa44036, 0xa67a69, 0xccb28f, 0xf8e2b9, 0xf8e6d6, 0xf8d6be, 0xe4937a, 0xbb4035, 0xa51914, 0xb3332d, 0xd67670, 0xe09f94, 0xfbccb1, 0xfee4d3, 0xf8e7dc, 0xf2dccf, 0xf4ebe2, 0xf1e0d9, 0xe1c4b6, 0xdec5bc, 0xd9b9ac, 0xd8bbb0, 0xd3b5ab, 0xcca89c, 0xceaca4, 0xe1c9bf, 0xedd9c3, 0xc2a57f, 0x7c2316, 0x882b21, 0x9b221d, 0xa82421, 0xc8645c, 0xecaf9d, 0xfbd9c0, 0xfcedde, 0xf0dbd0, 0xa3736a, 0xa3564b, 0xa7392b, 0xa3160d
            , 0xa72317, 0xa53c2c, 0xa5765f, 0xc1b090, 0xf9e3b8, 0xfbecdc, 0xfad0b7, 0xeb9c84, 0xbc4437, 0x9f0f0c, 0xb43830, 0xd07363, 0xe9b8a9, 0xf7dcce, 0xfeefda, 0xfff2e9, 0xfde4d7, 0xfddac7, 0xfdddd3, 0xfdded3, 0xfde5d9, 0xfee7dd, 0xfeeade, 0xfff4e9, 0xfcf6ed, 0xf1d8d1, 0xc57b75, 0x9d211c, 0x9e120f, 0xbf5850, 0xecae9e, 0xfbd8c0, 0xfdefd3, 0xe6cea4, 0xaa917a, 0xa45649, 0xa31b12, 0xa3160d
            , 0xa51d14, 0xa42c21, 0xaa745e, 0xc9b099, 0xfbe8d6, 0xfef6e9, 0xfbd6c3, 0xea9e86, 0xbe483a, 0xac231c, 0xa40e09, 0xae160d, 0xba1e11, 0xc12414, 0xc32413, 0xc32413, 0xc32413, 0xc32413, 0xc32413, 0xc42514, 0xc22313, 0xc02515, 0xb82014, 0xab1710, 0xa51916, 0xb7423b, 0xeca192, 0xfbdbcb, 0xfdeddd, 0xece1bc, 0xae9971, 0xa05a4c, 0xa31910, 0xa3160d
            , 0xa61f14, 0xab3626, 0xa46556, 0xc2a295, 0xfbc4ce, 0xfdefdc, 0xfde1cf, 0xedad98, 0xc04a3c, 0xa91f19, 0xad2722, 0xbf3e36, 0xd5382f, 0xe04039, 0xe4433b, 0xe4433b, 0xde3f37, 0xd53730, 0xc92c27, 0xb5211b, 0xaa1e18, 0xb63830, 0xeda892, 0xfdddc2, 0xfef2da, 0xe9eecf, 0xbaaa99, 0xa5624d, 0xa42317, 0xa3160d
            , 0xa92217, 0xa7271d, 0xa4584f, 0xc2999f, 0xefd2bc, 0xfef4e6, 0xfde5d7, 0xeeb39f, 0xc3584c, 0xa1120f, 0xaf1e19, 0xcc312c, 0xdf413c, 0xe2433f, 0xc62b28, 0xae1714, 0xa40f0d, 0xb82f2a, 0xe59480, 0xfcd5c0, 0xfceddd, 0xf3ddc8, 0xb5a595, 0xa88a7b, 0xa5281e, 0xa3180f
            , 0xa9241b, 0xa6271e, 0xa9584b, 0xc8ab8f, 0xfdedc1, 0xfef9f2, 0xfce5d6, 0xeaad9a, 0xbb4841, 0xa61814, 0xb21f1b, 0xbe2723, 0xa30f0c, 0xbe2c27, 0xe67568, 0xf8c9ba, 0xfcefda, 0xf5dfb8, 0xb19c85, 0xa58b6e, 0xa53023, 0xa3160d
            , 0xa72016, 0xa62b23, 0xa7584b, 0xc9ada5, 0xfef1ed, 0xfefaf2, 0xfad9c6, 0xe6a495, 0xbb4e48, 0xb83832, 0xd9685e, 0xf6b4a0, 0xfce4d1, 0xf6dbbd, 0xb7a196, 0xa5887a, 0xa73e31, 0xa3160d
            , 0xab2a1f, 0xa6261e, 0xa4615c, 0xc2ac94, 0xf5d7b7, 0xfdf2e3, 0xfde5d0, 0xfde3c7, 0xfae3c7, 0xf8ecae, 0xc7ba90, 0xa5746d, 0xa4221a, 0xa3160d
            , 0xa82218, 0xa3271d, 0xa26854, 0xbea588, 0xfde9c7, 0xfee2c4, 0xcaae98, 0xa87358, 0xa82b1f, 0xa3150c
            , 0xa41910, 0xa8392c, 0xa57e67, 0xa3533d, 0xa3180f, 0xa4170d
            , 0xa4170d, 0xa3150c, };


int image_init(void)
{
    data.enemy_data = (color_t *) malloc(ENEMY_DATA_SIZE);
    data.main_data = (color_t *) malloc(MAIN_DATA_SIZE);
    data.empire_data = (color_t *) malloc(EMPIRE_DATA_SIZE);
    data.tmp_data = (uint8_t *) malloc(SCRATCH_DATA_SIZE);
    if (!data.main_data || !data.empire_data || !data.enemy_data || !data.tmp_data) {
        free(data.main_data);
        free(data.empire_data);
        free(data.enemy_data);
        free(data.tmp_data);
        return 0;
    }
    return 1;
}

void image_enable_fonts(int enable)
{
    data.fonts_enabled = enable;
}

static void prepare_index(image *images, int size)
{
    int offset = 4;
    for (int i = 1; i < size; i++) {
        image *img = &images[i];
        if (img->draw.is_external) {
            if (!img->draw.offset) {
                img->draw.offset = 1;
            }
        } else {
            img->draw.offset = offset;
            offset += img->draw.data_length;
        }
    }
}

static void read_index_entry(buffer *buf, image *img)
{
    img->draw.offset = buffer_read_i32(buf);
    img->draw.data_length = buffer_read_i32(buf);
    img->draw.uncompressed_length = buffer_read_i32(buf);
    buffer_skip(buf, 8);
    img->width = buffer_read_u16(buf);
    img->height = buffer_read_u16(buf);
    buffer_skip(buf, 6);
    img->num_animation_sprites = buffer_read_u16(buf);
    buffer_skip(buf, 2);
    img->sprite_offset_x = buffer_read_i16(buf);
    img->sprite_offset_y = buffer_read_i16(buf);
    buffer_skip(buf, 10);
    img->animation_can_reverse = buffer_read_i8(buf);
    buffer_skip(buf, 1);
    img->draw.type = buffer_read_u8(buf);
    img->draw.is_fully_compressed = buffer_read_i8(buf);
    img->draw.is_external = buffer_read_i8(buf);
    img->draw.has_compressed_part = buffer_read_i8(buf);
    buffer_skip(buf, 2);
    img->draw.bitmap_id = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    img->animation_speed_id = buffer_read_u8(buf);
    buffer_skip(buf, 5);
}

static void read_index(buffer *buf, image *images, int size)
{
    for (int i = 0; i < size; i++) {
        read_index_entry(buf, &images[i]);
    }
    prepare_index(images, size);
}

static void read_header(buffer *buf)
{
    buffer_skip(buf, 80); // header integers
    for (int i = 0; i < 300; i++) {
        data.group_image_ids[i] = buffer_read_u16(buf);
    }
    buffer_read_raw(buf, data.bitmaps, 20000);
}

static color_t to_32_bit(uint16_t c)
{
    return ((c & 0x7c00) << 9) | ((c & 0x7000) << 4) |
           ((c & 0x3e0) << 6)  | ((c & 0x380) << 1) |
           ((c & 0x1f) << 3)   | ((c & 0x1c) >> 2);
}

static int convert_uncompressed(buffer *buf, int buf_length, color_t *dst)
{
    for (int i = 0; i < buf_length; i += 2) {
        *dst = to_32_bit(buffer_read_u16(buf));
        dst++;
    }
    return buf_length / 2;
}

static int convert_compressed(buffer *buf, int buf_length, color_t *dst)
{
    int dst_length = 0;
    while (buf_length > 0) {
        int control = buffer_read_u8(buf);
        if (control == 255) {
            // next byte = transparent pixels to skip
            *dst++ = 255;
            *dst++ = buffer_read_u8(buf);
            dst_length += 2;
            buf_length -= 2;
        } else {
            // control = number of concrete pixels
            *dst++ = control;
            for (int i = 0; i < control; i++) {
                *dst++ = to_32_bit(buffer_read_u16(buf));
            }
            dst_length += control + 1;
            buf_length -= control * 2 + 1;
        }
    }
    return dst_length;
}

static void convert_images(image *images, int size, buffer *buf, color_t *dst)
{
    color_t *start_dst = dst;
    dst++; // make sure img->offset > 0
    for (int i = 0; i < size; i++) {
        image *img = &images[i];
        if (img->draw.is_external) {
            continue;
        }
        buffer_set(buf, img->draw.offset);
        int img_offset = (int) (dst - start_dst);
        if (img->draw.is_fully_compressed) {
            dst += convert_compressed(buf, img->draw.data_length, dst);
        } else if (img->draw.has_compressed_part) { // isometric tile
            dst += convert_uncompressed(buf, img->draw.uncompressed_length, dst);
            dst += convert_compressed(buf, img->draw.data_length - img->draw.uncompressed_length, dst);
        } else {
            dst += convert_uncompressed(buf, img->draw.data_length, dst);
        }
        img->draw.offset = img_offset;
        img->draw.uncompressed_length /= 2;
    }
}

static void load_empire(void)
{
    int size = io_read_file_into_buffer(EMPIRE_555, MAY_BE_LOCALIZED, data.tmp_data, EMPIRE_DATA_SIZE);
    if (size != EMPIRE_DATA_SIZE / 2) {
        log_error("unable to load empire data", EMPIRE_555, 0);
        return;
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, size);
    convert_uncompressed(&buf, size, data.empire_data);
}

int image_load_climate(int climate_id, int is_editor, int force_reload)
{
    if (climate_id == data.current_climate && is_editor == data.is_editor && !force_reload) {
        return 1;
    }

    const char *filename_bmp = is_editor ? EDITOR_GRAPHICS_555[climate_id] : MAIN_GRAPHICS_555[climate_id];
    const char *filename_idx = is_editor ? EDITOR_GRAPHICS_SG2[climate_id] : MAIN_GRAPHICS_SG2[climate_id];

    if (MAIN_INDEX_SIZE != io_read_file_into_buffer(filename_idx, MAY_BE_LOCALIZED, data.tmp_data, MAIN_INDEX_SIZE)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, HEADER_SIZE);
    read_header(&buf);
    buffer_init(&buf, &data.tmp_data[HEADER_SIZE], ENTRY_SIZE * MAIN_ENTRIES);
    read_index(&buf, data.main, MAIN_ENTRIES);

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.main, MAIN_ENTRIES, &buf, data.main_data);
    data.current_climate = climate_id;
    data.is_editor = is_editor;

    load_empire();
    return 1;
}

static void free_font_memory(void)
{
    free(data.font);
    free(data.font_data);
    data.font = 0;
    data.font_data = 0;
    data.fonts_enabled = NO_EXTRA_FONT;
}

static int alloc_font_memory(int font_entries, int font_data_size)
{
    free_font_memory();
    data.font = (image*) malloc(font_entries * sizeof(image));
    data.font_data = (color_t *) malloc(font_data_size);
    if (!data.font || !data.font_data) {
        free(data.font);
        free(data.font_data);
        return 0;
    }
    memset(data.font, 0, font_entries * sizeof(image));
    return 1;
}

static int load_cyrillic_fonts(void)
{
    if (!alloc_font_memory(CYRILLIC_FONT_ENTRIES, CYRILLIC_FONT_DATA_SIZE)) {
        return 0;
    }
    if (CYRILLIC_FONT_INDEX_SIZE != io_read_file_part_into_buffer(CYRILLIC_FONTS_SG2, MAY_BE_LOCALIZED,
        data.tmp_data, CYRILLIC_FONT_INDEX_SIZE, CYRILLIC_FONT_INDEX_OFFSET)) {
        return 0;
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, CYRILLIC_FONT_INDEX_SIZE);
    read_index(&buf, data.font, CYRILLIC_FONT_ENTRIES);

    int data_size = io_read_file_into_buffer(CYRILLIC_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.font, CYRILLIC_FONT_ENTRIES, &buf, data.font_data);

    data.fonts_enabled = FULL_CHARSET_IN_FONT;
    data.font_base_offset = CYRILLIC_FONT_BASE_OFFSET;
    return 1;
}

static int parse_chinese_font(buffer *input, color_t *pixels, int pixel_offset, int char_size, int index_offset)
{
    int bytes_per_row = char_size <= 16 ? 2 : 3;
    for (int i = 0; i < IMAGE_FONT_MULTIBYTE_CHINESE_MAX_CHARS; i++) {
        image *img = &data.font[index_offset + i];
        img->width = char_size;
        img->height = char_size - 1;
        img->draw.bitmap_id = 0;
        img->draw.offset = pixel_offset;
        img->draw.uncompressed_length = img->draw.data_length = char_size * (char_size - 1);
        for (int row = 0; row < char_size - 1; row++) {
            unsigned int bits = buffer_read_u16(input);
            if (bytes_per_row == 3) {
                bits += buffer_read_u8(input) << 16;
            }
            int prev_set = 0;
            for (int col = 0; col < char_size; col++) {
                int set = bits & 1;
                if (set) {
                    *pixels = ALPHA_OPAQUE;
                } else if (prev_set) {
                    *pixels = ALPHA_FONT_SEMI_TRANSPARENT;
                } else {
                    *pixels = COLOR_SG2_TRANSPARENT;
                }
                pixels++;
                pixel_offset++;
                bits >>= 1;
                prev_set = set;
            }
        }
    }
    return pixel_offset;
}

static int load_traditional_chinese_fonts(void)
{
    if (!alloc_font_memory(TRAD_CHINESE_FONT_ENTRIES, TRAD_CHINESE_FONT_DATA_SIZE)) {
        return 0;
    }

    int data_size = io_read_file_into_buffer(TRAD_CHINESE_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer input;
    buffer_init(&input, data.tmp_data, data_size);
    color_t *pixels = data.font_data;
    int pixel_offset = 0;

    log_info("Parsing Chinese font", 0, 0);
    pixel_offset = parse_chinese_font(&input, &pixels[pixel_offset], pixel_offset, 12, 0);
    pixel_offset = parse_chinese_font(&input, &pixels[pixel_offset], pixel_offset, 16, IMAGE_FONT_MULTIBYTE_CHINESE_MAX_CHARS);
    pixel_offset = parse_chinese_font(&input, &pixels[pixel_offset], pixel_offset, 20, IMAGE_FONT_MULTIBYTE_CHINESE_MAX_CHARS * 2);
    log_info("Done parsing Chinese font", 0, 0);

    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

static int parse_korean_font(buffer *input, color_t *pixels, int pixel_offset, int char_size, int index_offset)
{
    int bytes_per_row = char_size <= 16 ? 2 : 3;
    for (int i = 0; i < IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS; i++) {
        image *img = &data.font[index_offset + i];
        img->width = char_size;
        img->height = char_size;
        img->draw.bitmap_id = 0;
        img->draw.offset = pixel_offset;
        img->draw.uncompressed_length = img->draw.data_length = char_size * char_size;
        for (int row = 0; row < char_size; row++) {
            unsigned int bits = buffer_read_u16(input);
            if (bytes_per_row == 3) {
                bits += buffer_read_u8(input) << 16;
            }
            int prev_set = 0;
            for (int col = 0; col < char_size; col++) {
                int set = bits & 1;
                if (set) {
                    *pixels = ALPHA_OPAQUE;
                } else if (prev_set) {
                    *pixels = ALPHA_FONT_SEMI_TRANSPARENT;
                } else {
                    *pixels = COLOR_SG2_TRANSPARENT;
                }
                pixels++;
                pixel_offset++;
                bits >>= 1;
                prev_set = set;
            }
        }
    }
    return pixel_offset;
}

static int load_korean_fonts(void)
{
    if (!alloc_font_memory(KOREAN_FONT_ENTRIES, KOREAN_FONT_DATA_SIZE)) {
        return 0;
    }

    int data_size = io_read_file_into_buffer(KOREAN_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        log_error("Julius requires extra files for Korean characters:", KOREAN_FONTS_555, 0);
        return 0;
    }
    buffer input;
    buffer_init(&input, data.tmp_data, data_size);
    color_t *pixels = data.font_data;
    int pixel_offset = 0;

    log_info("Parsing Korean font", 0, 0);
    pixel_offset = parse_korean_font(&input, &pixels[pixel_offset], pixel_offset, 12, 0);
    pixel_offset = parse_korean_font(&input, &pixels[pixel_offset], pixel_offset, 15, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS);
    pixel_offset = parse_korean_font(&input, &pixels[pixel_offset], pixel_offset, 20, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS * 2);
    log_info("Done parsing Korean font", 0, 0);

    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

int image_load_fonts(encoding_type encoding)
{
    if (encoding == ENCODING_CYRILLIC) {
        return load_cyrillic_fonts();
    } else if (encoding == ENCODING_TRADITIONAL_CHINESE) {
        return load_traditional_chinese_fonts();
    } else if (encoding == ENCODING_KOREAN) {
        return load_korean_fonts();
    } else {
        free_font_memory();
        return 1;
    }
}

int image_load_enemy(int enemy_id)
{
    const char *filename_bmp = ENEMY_GRAPHICS_555[enemy_id];
    const char *filename_idx = ENEMY_GRAPHICS_SG2[enemy_id];

    if (ENEMY_INDEX_SIZE != io_read_file_part_into_buffer(filename_idx, MAY_BE_LOCALIZED, data.tmp_data, ENEMY_INDEX_SIZE, ENEMY_INDEX_OFFSET)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, ENEMY_INDEX_SIZE);
    read_index(&buf, data.enemy, ENEMY_ENTRIES);

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.enemy, ENEMY_ENTRIES, &buf, data.enemy_data);
    return 1;
}

static const color_t *load_external_data(int image_id)
{
    image *img = &data.main[image_id];
    char filename[FILE_NAME_MAX] = "555/";
    strcpy(&filename[4], data.bitmaps[img->draw.bitmap_id]);
    file_change_extension(filename, "555");
    int size = io_read_file_part_into_buffer(
        &filename[4], MAY_BE_LOCALIZED, data.tmp_data,
        img->draw.data_length, img->draw.offset - 1
    );
    if (!size) {
        // try in 555 dir
        size = io_read_file_part_into_buffer(
            filename, MAY_BE_LOCALIZED, data.tmp_data,
            img->draw.data_length, img->draw.offset - 1
        );
        if (!size) {
            log_error("unable to load external image",
                      data.bitmaps[img->draw.bitmap_id], image_id);
            return NULL;
        }
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, size);
    color_t *dst = (color_t*) &data.tmp_data[4000000];
    // NB: isometric images are never external
    if (img->draw.is_fully_compressed) {
        convert_compressed(&buf, img->draw.data_length, dst);
    } else {
        convert_uncompressed(&buf, img->draw.data_length, dst);
    }
    return dst;
}

int image_group(int group)
{
    if (group == GROUP_ROADBLOCK) {
        return 10000;
    }
    return data.group_image_ids[group];
}

const image *image_get(int id)
{
    if (id == 10000) {
        return &roadblock_image;
    }
    if (id >= 0 && id < MAIN_ENTRIES) {
        return &data.main[id];
    } else {
        return NULL;
    }
}

const image *image_letter(int letter_id)
{
    if (data.fonts_enabled == FULL_CHARSET_IN_FONT) {
        return &data.font[data.font_base_offset + letter_id];
    } else if (data.fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.font[data.font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET];
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.main[data.group_image_ids[GROUP_FONT] + letter_id];
    } else {
        return &DUMMY_IMAGE;
    }
}

const image *image_get_enemy(int id)
{
    if (id >= 0 && id < ENEMY_ENTRIES) {
        return &data.enemy[id];
    } else {
        return NULL;
    }
}

const color_t *image_data(int id)
{
    if (id == 10000) {
        return roadblock_data;
    }
    if (id < 0 || id >= MAIN_ENTRIES) {
        return NULL;
    }
    if (!data.main[id].draw.is_external) {
        return &data.main_data[data.main[id].draw.offset];
    } else if (id == image_group(GROUP_EMPIRE_MAP)) {
        return data.empire_data;
    } else {
        return load_external_data(id);
    }
}

const color_t *image_data_letter(int letter_id)
{
    if (data.fonts_enabled == FULL_CHARSET_IN_FONT) {
        return &data.font_data[data.font[data.font_base_offset + letter_id].draw.offset];
    } else if (data.fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.font_data[data.font[data.font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET].draw.offset];
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        int image_id = data.group_image_ids[GROUP_FONT] + letter_id;
        return &data.main_data[data.main[image_id].draw.offset];
    } else {
        return NULL;
    }
}

const color_t *image_data_enemy(int id)
{
    if (data.enemy[id].draw.offset > 0) {
        return &data.enemy_data[data.enemy[id].draw.offset];
    }
    return NULL;
}
