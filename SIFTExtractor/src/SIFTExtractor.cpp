//============================================================================
// Name        : SiftExtractor.cpp
// Author      : nyoke
// Version     : 2.00
// Date		   : 2013/12/09
// Copyright   : nyoke all rights reserved.
// Description : openCVのライブラリを利用し，SIFT特徴量を算出
// 引数　　　　　 : SFITExtractor [image file] [feature file] [sampling: 0=DoG(Default), 1=DenceSampling]
//============================================================================
#include <iostream>
#include <string>
#include <fstream>
#include "SIFTExtractor.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	/* 引数のチェック */
	if (argc != 5)
	{
		cerr << "usage: SFITExtractor [image file] [feature file] [sampling: 0=DoG(Default), 1=DenceSampling] [scaling: 0=false(Default), 1=true]" << endl;
		return -1;
	}

	SIFTExtractor sift(argv[1], true);
	sift.extract(SIFTExtractor::dense);
	sift.save_feature(argv[2]);
	sift.save_image("result");

	return 0;
}
