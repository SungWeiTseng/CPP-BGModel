#pragma once
#define BIN_NUM 64			//128
#define THRESHOLD 0.15f	//0.07f
#define BETA 0.8f
#define ALPHA 1-BETA

#define NEIGHBORING
#define AVE_ARANGE 3
#define AVE_OFFSET AVE_ARANGE/2
#define CHANNEL_NUM 3
#define FG_DETERMINE 1

#define IMG_SIZE_X 416
#define IMG_SIZE_Y 416
//#define IMG_DIR "D:\\CoVision\\VideoSurvei\\imageData\\input1"//"D:\\CoVision\\VideoSurvei\\科技部計畫\\highway\\input"

#define FLOATING float
typedef FLOATING Float;
const int totalPixel = IMG_SIZE_X * IMG_SIZE_Y;
const int binWidth = 256 / BIN_NUM;
//const int HISTWIDTH = 768 / 3;
//const int HISTHEIGHT = 500;
