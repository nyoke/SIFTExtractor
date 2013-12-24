//============================================================================
// Name        : SiftExtractor.hpp
// Author      : nyoke
// Version     : 2.00
// Date		   : 2013/12/09
// Copyright   : nyoke all rights reserved.
// Description : openCVのライブラリを利用し，SIFT特徴量を算出するクラス
//============================================================================
#ifndef _SIFTEXTRACTOR_HPP_
#define _SIFTEXTRACTOR_HPP_

#include <iostream>
#include <string>
#include <fstream>
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace std;
using namespace cv;

/* declaration *****************************************************************/
class SIFTExtractor
{
public:
	enum sampling
	{
		dog, dense
	}; // サンプリング法 (dog = default, dense = dense sampling)
	SIFTExtractor(string);
	void extract(enum sampling, int feature_num = 100);
	void save_feature(string); 		// 特徴量の保存
	void save_image(string);	 	// 特徴量の抽出結果を保存

private:
	string image_file_name;   				// 入力画像のパス
	bool extraction;   				// 特徴量の抽出は完了したか？
	cv::Mat ref_image;						// 抽出対象原画像
	cv::Mat proc_image;						// 抽出対象画像
	std::vector<cv::KeyPoint> keypoints; 	// キーポイント
	cv::Mat descriptors;		 			// 抽出結果
	cv::SiftDescriptorExtractor extractor;  // 抽出機

	void extract_using_dog(int);    // DoGカーネルで特徴抽出
	void extract_using_dense(int);  // Dense Samplingカーネルで特徴抽出
};

/* Definition ******************************************************************/
SIFTExtractor::SIFTExtractor(string name)
{
	this->image_file_name = name; // 抽出対象のファイル名
	initModule_nonfree(); // non-freeライブラリを使う場合は必ず呼ぶ
	extraction = false;	  // 特徴量はまだ抽出されていない

	// (1) load Reference Image
	ref_image = imread(image_file_name);
	if (ref_image.empty())
		cerr << "cannot open ref_image file: " << image_file_name << endl; return;

	// (2) convert Reference Image to Gray-scale for Feature Extraction
	cv::cvtColor(ref_image, proc_image, CV_BGR2GRAY);
}


/* 特徴量の抽出（指定したカーネルに対応したメソッド呼び出し） */
void SIFTExtractor::extract(sampling kernel, int feature_num)
{
	switch (kernel)
	{
		case SIFTExtractor::dog:
			extract_using_dog(feature_num);
			break;
		case SIFTExtractor::dense:
			extract_using_dense(feature_num);
			break;
	}
}

/* DoGカーネルによる特徴量の抽出 */
void SIFTExtractor::extract_using_dog(int feature_num)
{

}

/* DoGカーネルによる特徴量の抽出 */
void SIFTExtractor::extract_using_dense(int feature_num)
{
	/*+ パラメータの算出 ++++++++++++++++*/
	Size image_size = Size(ref_image.cols, ref_image.rows); // 画像サイズの取得

	// 1. サンプリング間隔を求める
	double interval = sqrt((image_size.width * image_size.height) / feature_num);

	// 2. スケールの決定（サンプリング間隔 interval / 2.0)
	double scale = interval / 2.0;

	// 3. 抽出できるサンプル数の算出
	int sample_col_num = image_size.width / ceil(interval);
	int sample_row_num = image_size.height / ceil(interval);

	// 4. 画像の余白を算出
	int odd_cols = image_size.width - (sample_col_num * ceil(interval));
	int odd_rows = image_size.width - (sample_row_num * ceil(interval));

	// 5. 左上から何pixelシフトした位置から特徴量を抽出するかを算出
	double sift = sqrt((odd_cols * odd_rows) / 4);

	/*+ パラメータの算出 ++++++++++++++++*/
	// 1. 半径 scale[pixel]の大きさの特徴点を interval[pixel]間隔でサンプリングする
	cv::DenseFeatureDetector detector(ceil(scale), 1, 0.1f, ceil(interval), int(sift + 0.5), true, false);
	detector.detect(ref_image, keypoints);

	// 2. SIFT記述子の抽出
	extractor.compute(ref_image, keypoints, descriptors);

	//TODO: エラー処理

	// 抽出済みフラグをセット
	extraction = true;
}

/* 特徴量の書き出し */
void SIFTExtractor::save_feature(string file_name)
{
	if(!extraction)
	{
		cout << "image features could not extract yet." << endl;
		return;
	}

	//　書き出しファイルのオープン
	fstream fout(file_name.c_str(), ios::out);
	if (!fout.is_open())
	{
		cerr << "cannot open feature file: " << file_name << "¥n";
		return;
	}

	// 出力精度の設定（小数点以下6桁）
	fout.setf(ios::fixed);
	fout.precision(6);

	//一行目にキーポイント数と次元数を書き込む
	fout << keypoints.size() << "\t" << extractor.descriptorSize();
	fout << endl;

	// 特徴点を書き出す．
	std::vector<cv::KeyPoint>::iterator itk;
	int keypoint_index = 0;
	for (itk = keypoints.begin(); itk != keypoints.end(); ++itk, keypoint_index++)
	{
		// (1) キーポイント情報の出力
		fout << itk->pt.x << "\t";  // x座標
		fout << itk->pt.y << "\t";  // y座標
		fout << itk->size << "\t";  // スケール
		fout << itk->angle << "\t"; // 向き

		//(2) 特徴ベクトルの出力
		for (int dim_index = 0; dim_index < extractor.descriptorSize(); dim_index++)
			fout << (int) (descriptors.at<float>(keypoint_index, dim_index)) << "\t";

		fout << endl;
	}
	fout.close();
}

void SIFTExtractor::save_image(string file_name)
{
	if(!extraction)
	{
		cout << "image features could not extract yet." << endl;
		return;
	}

	// 特徴点を画像に書き出す．
	std::vector<cv::KeyPoint>::iterator itk;
	int keypoint_index = 0;
	for (itk = keypoints.begin(); itk != keypoints.end(); ++itk, keypoint_index++)
	{
		cv::circle(ref_image, itk->pt, itk->size, cv::Scalar(0, 255, 255), 1, CV_AA);
		cv::circle(ref_image, itk->pt, 1, cv::Scalar(0, 255, 0), -1);
	}

	string save = file_name;
	save.append("_sift.png");

	// Write image file.
	cv::imwrite(save, ref_image);
}

#endif //_SIFTEXTRACTOR_HPP_
