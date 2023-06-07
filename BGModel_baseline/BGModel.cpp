#define _CRT_SECURE_NO_WARNINGS
#include "BGModel.h"
#include <iostream>
#include <highgui.h>
#include <opencv.hpp>
#include <ctime>

using namespace std;
using namespace cv;

struct Block
{
	double binY [NUM_BIN];
	double binCr[NUM_BIN];
	double binCb[NUM_BIN];

	bool flagY[NUM_BIN];
	bool flagCr[NUM_BIN];
	bool flagCb[NUM_BIN];

	Block() {
		memset(binY , 0, sizeof(binY));
		memset(binCr, 0, sizeof(binCr));
		memset(binCb, 0, sizeof(binCb));
		resetFlag();
	}

	void resetFlag() {
		memset(flagY, 0, sizeof(flagY));
		memset(flagCr, 0, sizeof(flagCr));
		memset(flagCb, 0, sizeof(flagCb));
	}

	void decrease() {
		for (int i = 0; i < NUM_BIN; i++) binY[i] *= (1 - ALPHA);
		for (int i = 0; i < NUM_BIN; i++) binCr[i] *= (1 - ALPHA);
		for (int i = 0; i < NUM_BIN; i++) binCb[i] *= (1 - ALPHA);
	}

	void increase() {

		for (int i = 0; i < NUM_BIN; i++)
		{
#ifdef NEIGHBOR
			double lr = ALPHA;
			if (flagY[i]) {

				binY[i] += lr;
				for (int j = 1; j <= DELTA_Y / 2; j++) {
					lr /= 2;
					if (i + j < NUM_BIN) binY[i + j] += lr;
					if (i - j >= 0)		 binY[i - j] += lr;
				}
			}
			//if (flagY[i]) {
			//	for (int X = 0; X < NUM_BIN / 2; X++) {
			//		double LR = 1 / (DELTA_Y * sqrt(2 * PI));
			//		LR *= exp(-(X * X) / (2 * DELTA_Y * DELTA_Y));
			//		if (i + X < NUM_BIN) binY[i + X] += (ALPHA * LR);
			//		if (i - X >= 0)binY[i - X] += (ALPHA * LR);
			//	}
			//}
#else
			if (flagY[i]) binY[i] += ALPHA;
#endif
		}
		for (int i = 0; i < NUM_BIN; i++)
		{
#ifdef NEIGHBOR
			double lr = ALPHA;
			if (flagCr[i]) {
				//lr /= 2;
				binCr[i] += lr;
				for (int j = 1; j <= DELTA_Cr / 2; j++) {
					lr /= 2;
					if (i + j < NUM_BIN) binCr[i + j] += lr;
					if (i - j >= 0)		 binCr[i - j] += lr;
				}
			}
			//#endif
			//#ifdef Gaussian
			//if (flagCr[i]) {
			//	for (int X = 0; X < NUM_BIN / 2; X++) {
			//		double LR = 1 / (DELTA_Cr * sqrt(2 * PI));
			//		LR *= exp(-(X * X) / (2 * DELTA_Cr * DELTA_Cr));
			//		if (i + X < NUM_BIN) binCr[i + X] += (ALPHA * LR);
			//		if (i - X >= 0)binCr[i - X] += (ALPHA * LR);
			//	}
			//}
#else
			if (flagCr[i]) binCr[i] += ALPHA;
#endif
		}
		for (int i = 0; i < NUM_BIN; i++)
		{
#ifdef NEIGHBOR
			double lr = ALPHA;
			if (flagCb[i]) {
				//lr /= 2;

				binCb[i] += lr;
				for (int j = 1; j <= DELTA_Cb / 2; j++) {
					lr /= 2;
					if (i + j < NUM_BIN) binCb[i + j] += lr;
					if (i - j >= 0)		 binCb[i - j] += lr;
				}
			}
			//#endif
			//#ifdef Gaussian
			//if (flagCb[i]) {
			//	for (int X = 0; X < NUM_BIN / 2; X++) {
			//		double LR = 1 / (DELTA_Cb * sqrt(2 * PI));
			//		LR *= exp(-(X * X) / (2 * DELTA_Cb * DELTA_Cb));
			//		if (i + X < NUM_BIN) binCb[i + X] += (ALPHA * LR);
			//		if (i - X >= 0)binCb[i - X] += (ALPHA * LR);
			//	}
			//}
#else
			if (flagCb[i]) binCb[i] += ALPHA;
#endif
		}
		resetFlag();
	}
};

class BGModel
{
private:
	int lookUpTable[256];
	unsigned char* FGMap;
	Block** blocks;

public:

	BGModel() {
		FGMap = new unsigned char[IMG_SIZE_X * IMG_SIZE_X];
		blocks = new Block*[NUM_MODEL_Y];
		for (int i = 0; i < NUM_MODEL_Y; i++) {
			blocks[i] = new Block[NUM_MODEL_X];
		}
		for (int i = 0; i < 256; i++) {
			lookUpTable[i] = i / BIN_WIDTH;
		}
	}
	~BGModel() {
		delete[] FGMap;

		for (int i = 0; i < NUM_MODEL_Y; i++)
			delete[] blocks[i];
		delete[] blocks;

		blocks = NULL;
		FGMap = NULL;
	}
	void Initial(unsigned char* Img) {

		for (int blk_y = 0; blk_y < NUM_MODEL_Y; blk_y++) {
			for (int blk_x = 0; blk_x < NUM_MODEL_X; blk_x++) {
				for (int img_y = 0; img_y < BLOCK_SIZE; img_y++) {
					for (int img_x = 0; img_x < BLOCK_SIZE; img_x++) {
						int idx = (blk_y * BLOCK_SIZE + img_y) * IMG_SIZE_X + blk_x * BLOCK_SIZE + img_x;
						int Y = idx * 3 + 0;
						int Cr = idx * 3 + 1;
						int Cb = idx * 3 + 2;

						blocks[blk_y][blk_x].binY[lookUpTable[Img[Y]]] = 1;
						blocks[blk_y][blk_x].binCr[lookUpTable[Img[Cr]]] = 1;
						blocks[blk_y][blk_x].binCb[lookUpTable[Img[Cb]]] = 1;
					}
				}
			}
		}
	}
	void Update(unsigned char* Img) {
		memset(&FGMap[0], 0, IMG_SIZE_X * IMG_SIZE_X);
		for (int blk_y = 0; blk_y < NUM_MODEL_Y; blk_y++) {
			for (int blk_x = 0; blk_x < NUM_MODEL_X; blk_x++) {
				for (int img_y = 0; img_y < BLOCK_SIZE; img_y++) {
					for (int img_x = 0; img_x < BLOCK_SIZE; img_x++) {
						int idx = (blk_y * BLOCK_SIZE + img_y) * IMG_SIZE_X + blk_x * BLOCK_SIZE + img_x;
						int Y = idx * 3 + 0;
						int Cr = idx * 3 + 1;
						int Cb = idx * 3 + 2;

						int Bin = lookUpTable[Img[Y]];
						if (!blocks[blk_y][blk_x].flagY[Bin])
							blocks[blk_y][blk_x].flagY[Bin] = true;
#ifdef NEIGHBOR2
						float sum = blocks[blk_y][blk_x].binY[Bin];
						for (int j = 1; j <= DELTA_Y / 2; j++) {
							if (Bin + j < NUM_BIN) sum += blocks[blk_y][blk_x].binY[Bin + j];
							if (Bin - j >= 0)	   sum += blocks[blk_y][blk_x].binY[Bin - j];
						}

						if (sum < THRESHOLD)
							FGMap[idx] = 255;
#else
						if (blocks[blk_y][blk_x].binY[Bin] < (THRESHOLD / (BLOCK_SIZE / 2)))
							FGMap[idx] = 255;
#endif // NEIGHBOR2			

						Bin = lookUpTable[Img[Cr]];
						if (!blocks[blk_y][blk_x].flagCr[Bin])
							blocks[blk_y][blk_x].flagCr[Bin] = true;
#ifdef NEIGHBOR2
						sum = blocks[blk_y][blk_x].binCr[Bin];
						for (int j = 1; j <= DELTA_Cr / 2; j++) {
							if (Bin + j < NUM_BIN) sum += blocks[blk_y][blk_x].binCr[Bin + j];
							if (Bin - j >= 0)	   sum += blocks[blk_y][blk_x].binCr[Bin - j];
						}

						if (sum < THRESHOLD)
							FGMap[idx] = 255;
#else
						if (blocks[blk_y][blk_x].binCr[Bin] < THRESHOLD)
							FGMap[idx] = 255;
#endif
						Bin = lookUpTable[Img[Cb]];
						if (!blocks[blk_y][blk_x].flagCb[Bin])
							blocks[blk_y][blk_x].flagCb[Bin] = true;
#ifdef NEIGHBOR2
						sum = blocks[blk_y][blk_x].binCb[Bin];
						for (int j = 1; j <= DELTA_Cb / 2; j++) {
							if (Bin + j < NUM_BIN) sum += blocks[blk_y][blk_x].binCb[Bin + j];
							if (Bin - j >= 0)	   sum += blocks[blk_y][blk_x].binCb[Bin - j];
						}

						if (sum < THRESHOLD)
							FGMap[idx] = 255;
#else
						if (blocks[blk_y][blk_x].binCb[Bin] < THRESHOLD)
							FGMap[idx] = 255;
#endif
					}
				}
				blocks[blk_y][blk_x].decrease();
				blocks[blk_y][blk_x].increase();
			}
		}

	}
	unsigned char* getFG() {
		return FGMap;
	}
};

int main() {

	double p = 0;
	for (int i = 0; i < update_times; i++) {
		p = p * (1 - ALPHA) + ALPHA;
	}
	THRESHOLD = p * BIN_WIDTH * 3;
	cout << THRESHOLD << endl;

	clock_t a, b;
	clock_t time = 0;

	BGModel Model;
	Size dsize = Size(IMG_SIZE_X, IMG_SIZE_Y);

	Mat InputImg = Mat(dsize, CV_8UC3);
	Mat OutputImg(IMG_SIZE_Y, IMG_SIZE_X, CV_8U, Scalar(0, 0, 0));
	Mat resizeImg = Mat(dsize, CV_8UC3);

	char fileName[128] = { '\0' };
	int count = 1;
	int totalCount = 0;
	sprintf(fileName, "D:\\CoVision\\IMAGE\\copyMachine\\input\\in000001.jpg");
	Mat srcImg = imread(fileName);
	resize(srcImg, InputImg, dsize);
	cvtColor(InputImg, resizeImg, COLOR_BGR2YCrCb);

	Model.Initial(resizeImg.data);

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	a = clock();
	while (count < 3400) {
		//cout << count << endl;
		sprintf(fileName, "D:\\CoVision\\IMAGE\\copyMachine\\input\\in%06d.jpg", count);
		count++;

		srcImg = imread(fileName);
		resize(srcImg, InputImg, dsize);
		cvtColor(InputImg, resizeImg, COLOR_BGR2YCrCb);

		Model.Update(resizeImg.data);

		OutputImg.data = Model.getFG();
		//sprintf(fileName, "D:\\CoVision\\IMAGE\\copyMachine\\copyMachine\\segmentation\\in%06d.jpg", count);
		//imwrite(fileName, OutputImg);
		//morphologyEx(OutputImg, OutputImg, MORPH_OPEN, element);
		imshow("Input", InputImg);
		imshow("Foreground", OutputImg);
		cvWaitKey(1);
		if (count == 3400) count = 1;
	}
	b = clock();
	time += (b - a);
	cout << 3400 / (double(time) / CLOCKS_PER_SEC) << endl;
	return 0;
}