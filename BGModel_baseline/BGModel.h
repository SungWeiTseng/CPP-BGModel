#pragma once

#define NEIGHBOR
#define NEIGHBOR2
#define LR_SCALE 2
#define NUM_BIN 64

#define IMG_SIZE_X 640
#define IMG_SIZE_Y 480
#define BLOCK_SIZE 8

#define FPS 30
#define HOLDON 10

const int update_times = FPS * HOLDON;
const int THR_SCALE = .5;//NUM_BIN / 32;
const int BIN_WIDTH = 256 / NUM_BIN;
const float ALPHA = 1.0 / (update_times * 3 * BIN_WIDTH);
const int DELTA_Y  = NUM_BIN / 16;
const int DELTA_Cr = NUM_BIN / 32;
const int DELTA_Cb = NUM_BIN / 32;
float THRESHOLD;

const int NUM_MODEL_X = IMG_SIZE_X / BLOCK_SIZE;
const int NUM_MODEL_Y = IMG_SIZE_Y / BLOCK_SIZE;