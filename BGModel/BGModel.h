#pragma once
#include<pybind11/pybind11.h>
#include<pybind11/numpy.h>
#include "DataDef.h"

extern int lookUpTable[256];
using namespace pybind11;

//typedef struct Point {
//	int x = -1;
//	int y = -1;
//}point;


typedef struct binBlock {
	Float Bin[BIN_NUM]{ 0 };
	Float beta = BETA;
	Float alpha = ALPHA;
}Block;

class BGModel {
private:
	array_t<int> FGMap;
	array_t<int> FG;
	array_t<Float> FGProb;
	Block** blocks;
	buffer_info fgMapBuf;
	buffer_info fgBuf;
public:
	BGModel();
	~BGModel();
	void free();
	array_t<Float> input(array_t<int>, int);
	void updateInput(array_t<int>, int);
	void init(array_t<int>, int);
	void LR_update(array_t<int>, array_t<int>, int);
	void gtUpdate(array_t<int>, array_t<int>, int);
	array_t<int> getFG();
	array_t<int> getBG();
};

PYBIND11_MODULE(BGModel, m) {
	class_<BGModel>(m, "BGModel")
		.def(init<>())
		.def("init", &BGModel::init)
		.def("LR_update", &BGModel::LR_update)
		.def("input", &BGModel::input)
		.def("getFG", &BGModel::getFG)
		.def("updateInput", &BGModel::updateInput)
		.def("gtUpdate", &BGModel::gtUpdate)
		.def("getBG", &BGModel::getBG);
}