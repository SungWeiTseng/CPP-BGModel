#include "BGModel.h"
#include <assert.h>
#include <iostream>
#include <fstream>
using namespace std;

int lookUpTable[256];

BGModel::BGModel() {
	blocks = new Block*[CHANNEL_NUM];
	FGMap = array_t<int>(totalPixel * 3);
	FG = array_t<int>(totalPixel);
	FGProb = array_t<Float>(totalPixel);
	fgMapBuf = FGMap.request();
	fgBuf = FG.request();
	for (int i = 0; i < CHANNEL_NUM; i++) {
		blocks[i] = new Block[totalPixel];
	}
	for (int i = 0; i < 256; i++) {
		lookUpTable[i] = i / binWidth;
	}
};

BGModel::~BGModel() {
	free();
}

void BGModel::updateInput(array_t<int> img, int CHAN) {
	buffer_info FGProbBuf = FGProb.request();
	buffer_info imgBuf = img.request();
	int* imgPtr = (int*)imgBuf.ptr;
	int* fgMapPtr = (int*)fgMapBuf.ptr;
	Float* FGProPtr = (Float*)FGProbBuf.ptr;
	memset(&fgMapPtr[CHAN*totalPixel], 0, sizeof(int)*totalPixel);

	Block* blkPtr = blocks[CHAN];
	for (int idx = 0; idx < totalPixel; idx++, imgPtr++, blkPtr++) {
		int k = lookUpTable[*imgPtr];
#ifdef NEIGHBORING
		Float sum = 0;
		if (k < AVE_OFFSET) {
			for (int i = 0; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k + i];
			if (sum / (AVE_OFFSET + 1) < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
		else if (k > BIN_NUM - AVE_OFFSET - 1) {
			for (int i = 0; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k - i];
			if (sum / (AVE_OFFSET + 1) < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
		else {
			for (int i = -AVE_OFFSET; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k + i];
			if (sum / AVE_ARANGE < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
#else
		if (blkPtr->Bin[k] < THRESHOLD)
			FGMap[CHAN][idx] = 255;
#endif
		for (int n = 0; n < BIN_NUM; n++) {
			blkPtr->Bin[n] *= blkPtr->beta;
		}
		blkPtr->Bin[k] += blkPtr->alpha;
	}
}

void BGModel::LR_update(array_t<int> img, array_t<int> LRMap, int c) {
	buffer_info mapBuf = LRMap.request();
	buffer_info imgBuf = img.request();
	int* imgPtr = (int*)imgBuf.ptr;
	int* mapPtr = (int*)mapBuf.ptr;
	Block* blkPtr = blocks[c];

	for (int idx = 0; idx < totalPixel; idx++, imgPtr++, blkPtr++, mapPtr++) {
		int k = lookUpTable[*imgPtr];
		Float r;

		switch ((*mapPtr)) {
		case 0:
			blkPtr->alpha = ALPHA;
			blkPtr->beta = BETA;
			break;
		case 2://¤H
			r = blkPtr->alpha = 0;
			blkPtr->beta = 1 - r;

			/*r = (-0.001) / (1 - blkPtr->Bin[k]);
			blkPtr->beta = 1 - r;*/
			break;
		default:
			r = blkPtr->alpha = (THRESHOLD + 0.02 - blkPtr->Bin[k]) / (1 - blkPtr->Bin[k]);
			blkPtr->beta = 1 - r;
			break;
		}

		for (int n = 0; n < BIN_NUM; n++) {
			blkPtr->Bin[n] *= blkPtr->beta;
		}
		blkPtr->Bin[k] += blkPtr->alpha;
	}

}

array_t<int> BGModel::getFG() {
	int *fgMapPtr = (int*)fgMapBuf.ptr;
	int *fgPtr = (int*)fgBuf.ptr;
	memset((int*)fgBuf.ptr, 0, totalPixel * sizeof(int));
	for (int i = 0; i < CHANNEL_NUM; i++) {
		for (int j = 0; j < totalPixel; j++) {
			if (fgMapPtr[i*totalPixel + j] == 255)
				fgPtr[j] = 255;

			/*if (fgPtr[j] == 255) continue;
			if (fgMapPtr[i*totalPixel + j] == 255) {
				fgPtr[j]++;
				if (fgPtr[j] >= FG_DETERMINE)
					fgPtr[j] = 255;
			}
			else if (i == CHANNEL_NUM - 1) {
				fgPtr[j] = 0;
			}*/
		}
	}
	return FG;
}

array_t<Float> BGModel::input(array_t<int> img, int CHAN) {

	buffer_info FGProbBuf = FGProb.request();
	buffer_info imgBuf = img.request();
	int* imgPtr = (int*)imgBuf.ptr;
	int *fgMapPtr = (int*)fgMapBuf.ptr;
	Float* FGProPtr = (Float*)FGProbBuf.ptr;
	memset(&fgMapPtr[CHAN*totalPixel], 0, sizeof(int)*totalPixel);
	memset(FGProPtr, 0, sizeof(int)*totalPixel);

	Block* blkPtr = blocks[CHAN];
	for (int idx = 0; idx < totalPixel; idx++, imgPtr++, blkPtr++) {
		int k = lookUpTable[*imgPtr];
#ifdef NEIGHBORING
		Float sum = 0;
		if (k < AVE_OFFSET) {
			for (int i = 0; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k + i];
			if (sum / (AVE_OFFSET + 1) < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
		else if (k > BIN_NUM - AVE_OFFSET - 1) {
			for (int i = 0; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k - i];
			if (sum / (AVE_OFFSET + 1) < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
		else {
			for (int i = -AVE_OFFSET; i <= AVE_OFFSET; i++)
				sum += blkPtr->Bin[k + i];
			if (sum / AVE_ARANGE < THRESHOLD)
				fgMapPtr[CHAN*totalPixel + idx] = 255;
		}
#else
		if (blkPtr->Bin[k] < THRESHOLD)
			FGMap[CHAN][idx] = 255;
#endif
		FGProPtr[idx] = blkPtr->Bin[k];
	}
	return FGProb;
}

void BGModel::gtUpdate(array_t<int> img, array_t<int> gt, int CHAN) {
	buffer_info gtBuf = gt.request();
	buffer_info imgBuf = img.request();
	int* imgPtr = (int*)imgBuf.ptr;
	int* gtPtr = (int*)gtBuf.ptr;
	Block* blkPtr = blocks[CHAN];

	for (int idx = 0; idx < totalPixel; idx++, imgPtr++, blkPtr++, gtPtr++) {
		int k = lookUpTable[*imgPtr];
		Float r;

		if ((*gtPtr) != 255) {
			for (int n = 0; n < BIN_NUM; n++) {
				blkPtr->Bin[n] *= blkPtr->beta;
			}
			blkPtr->Bin[k] += blkPtr->alpha;
		}
	}
}

void BGModel::free() {
	for (int i = 0; i < CHANNEL_NUM; i++) {
		delete[] blocks[i];
	}
	delete[] blocks;
	blocks = NULL;
}

void BGModel::init(array_t<int> img, int CHAN) {
	buffer_info imgBuf = img.request();
	int* imgPtr = (int*)imgBuf.ptr;
	Block* blkPtr = blocks[CHAN];
	for (int i = 0; i < totalPixel; i++, blkPtr++, imgPtr++) {
		memset(blkPtr->Bin, 0, sizeof(Float)*BIN_NUM);
		blkPtr->Bin[lookUpTable[*imgPtr]] = 1;
	}
}

array_t<int> BGModel::getBG() {
	array_t<int> BG;
	BG = array_t<int>(totalPixel*CHANNEL_NUM);
	buffer_info BGBuf = BG.request();
	int *BGBufx = (int*)BGBuf.ptr;


	for (int i = 0; i < CHANNEL_NUM; i++) {
		Block* blkPtr = blocks[i];
		for (int j = 0; j < totalPixel; j++, blkPtr++) {
			int max = 0;
			int maxidx = -1;
			for (int k = 0; k < BIN_NUM; k++) {
				if (blkPtr->Bin[k] > max) {
					max = blkPtr->Bin[k];
					maxidx = k;
				}
			}
			BGBufx[i + j * 3] = maxidx*4+2;
		}
	}
	return BG;
}