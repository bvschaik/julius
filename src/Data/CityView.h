#ifndef DATA_CITYVIEW_H
#define DATA_CITYVIEW_H

#define VIEW_X_MAX 165
#define VIEW_Y_MAX 325

extern struct _Data_CityView {
	int viewToGridOffsetLookup[VIEW_X_MAX][VIEW_Y_MAX];

	// frequently updated
	struct {
		int xOffsetInPixels;
		int yOffsetInPixels;
	} selectedTile;
} Data_CityView;

#define ViewToGridOffset(x,y) (Data_CityView.viewToGridOffsetLookup[x][y])

#endif
