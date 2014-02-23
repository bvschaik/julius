#ifndef DATA_CITYVIEW_H
#define DATA_CITYVIEW_H

#define VIEW_X_MAX 165
#define VIEW_Y_MAX 325

extern struct Data_CityView {
	int xOffsetInPixels;
	int yOffsetInPixels;
	int widthInPixels;
	int heightInPixels;

	int xInTiles;
	int yInTiles;
	int widthInTiles;
	int heightInTiles;

	int viewToGridOffsetLookup[VIEW_X_MAX][VIEW_Y_MAX];
} Data_CityView;

#define ViewToGridOffset(x,y) (Data_CityView.viewToGridOffsetLookup[x][y])

#endif
