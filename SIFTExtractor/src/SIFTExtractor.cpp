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
	if (argc != 6)
	{
		cerr << "usage: SFITExtractor [image file] [feature file] [feature_num] [sampling: 0=DoG, 1=DenceSampling] [scaling: 0=false, 1=true]" << endl;
		return -1;
	}

	// スケーリング引数のチェック
	if( (atoi(argv[5]) != 0) && (atoi(argv[5]) != 1))
	{
		cerr << "パラメータ指定が違います(scaling)" << endl;
		return -1;
	}

	SIFTExtractor sift(argv[1], atoi(argv[5]), atoi(argv[3]));

	if(atoi(argv[4]) == 0)
		sift.extract(SIFTExtractor::dog);
	else if(atoi(argv[4]) == 1)
		sift.extract(SIFTExtractor::dense);

	sift.save_feature(argv[2]);
	sift.save_image("result");

	return 0;
}
