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
	SIFTExtractor(string, bool resize, int feature_num = 100);
	void extract(enum sampling);
	void save_feature(string); 		// 特徴量の保存
	void save_image(string);	 	// 特徴量の抽出結果を保存

private:
	string image_file_name;   				// 入力画像のパス
	bool extraction;   						// 特徴量の抽出は完了したか？
	bool resize;							// 正方形にリサイズするか？
	int feature_num;						// 抽出する特徴量の数
	cv::Mat ref_image;						// 抽出対象原画像
	cv::Mat proc_image;						// 抽出対象画像
	std::vector<cv::KeyPoint> keypoints; 	// キーポイント
	cv::Mat descriptors;		 			// 抽出結果
	cv::SiftDescriptorExtractor extractor;  // 抽出機
	double scale_x;							// x方向のスケーリングパラメータ
	double scale_y;							// y方向のスケーリングパラメータ

	void extract_using_dog();    // DoGカーネルで特徴抽出
	void extract_using_dense();  // Dense Samplingカーネルで特徴抽出
	void resize_image(cv::Mat &);			// imageを正方形にリサイズする
};

/* Definition ******************************************************************/
SIFTExtractor::SIFTExtractor(string name, bool resize, int feature_num)
{
	image_file_name = name;	 // 抽出対象のファイル名
	initModule_nonfree();	 // non-freeライブラリを使う場合は必ず呼ぶ
	extraction = false;	  	 // 特徴量の抽出完了フラグを未抽出(false)に初期化
	this->resize = resize; 	 // リサイズの有無
	this->feature_num = feature_num; // 特徴量の抽出数
	scale_x = scale_y = 1.0; // スケーリングパラメータの初期化（初期値=等倍）

	// (1) load Reference Image
	ref_image = imread(image_file_name);
	if (ref_image.empty())
	{
		cerr << "cannot open ref_image file: " << image_file_name << endl;
		return;
	}

	// (2) convert Reference Image to Gray-scale for Feature Extraction
	proc_image = Mat::zeros(ref_image.cols, ref_image.rows, CV_8UC1);
	cvtColor(ref_image, proc_image, CV_BGR2GRAY);

	// リサイズオプションがある場合は正方形にリサイズする
	if(resize == true)
		resize_image(proc_image);
}


/* 特徴量の抽出（指定したカーネルに対応したメソッド呼び出し） */
void SIFTExtractor::extract(sampling kernel)
{
	switch (kernel)
	{
		case SIFTExtractor::dog:
			extract_using_dog();
			break;
		case SIFTExtractor::dense:
			extract_using_dense();
			break;
	}
}

/* DoGカーネルによる特徴量の抽出 */
void SIFTExtractor::extract_using_dog()
{

}

/* DoGカーネルによる特徴量の抽出 */
void SIFTExtractor::extract_using_dense()
{
	/*+ パラメータの算出 ++++++++++++++++*/
	Size image_size = Size(proc_image.cols, proc_image.rows); // 画像サイズの取得

	// 1. サンプリング間隔を求める
	double interval = sqrt((image_size.width * image_size.height) / (double)feature_num);

	// 2. スケールの決定（サンプリング間隔 interval / 2.0)
	double scale = interval / 2.0;

	// 3. interval間隔で横・縦ともに抽出できるサンプル数の算出
	int sample_col_num = (double)image_size.width  / floor(interval);
	int sample_row_num = (double)image_size.height / floor(interval);

	// 4. 画像の余白を算出
	int odd_cols, odd_rows;

	if( (image_size.width % sample_col_num == 0) && (image_size.height % sample_row_num == 0) )
	{
		//cout << "1"  << endl;
		odd_cols = image_size.width -  ( (sample_col_num - 1)  * floor(interval) );
		odd_rows = image_size.height - ( (sample_row_num - 1) * floor(interval) );
	}
	else if( (image_size.width % sample_col_num == 0) && (image_size.height % sample_row_num != 0) )
	{
		//cout << "2"  << endl;
		odd_cols = image_size.width -  ( (sample_col_num - 1) * floor(interval) );
		odd_rows = image_size.height - ( sample_row_num * floor(interval) );
	}
	else if( (image_size.width % sample_col_num == 0) && (image_size.height % sample_row_num == 0) )
	{
		//cout << "3"  << endl;
		odd_cols = image_size.width -  ( sample_col_num * floor(interval) );
		odd_rows = image_size.height - ( (sample_row_num - 1) * floor(interval) );
	}
	else
	{
		//cout << "4"  << endl;
		odd_cols = image_size.width -  ( sample_col_num * floor(interval) );
		odd_rows = image_size.height - ( sample_row_num * floor(interval) );
	}

	// 5. 左上から何pixelシフトした位置から特徴量を抽出するかを算出
	double sift = sqrt((odd_cols * odd_rows) / 4.0);

	// 6. 正方形の画像の場合の例外処理
	if( image_size.width == image_size.height)
	{
		int tmp = (double)image_size.width - (floor(sift) * 2.0) - (floor(interval) * (sqrt(feature_num) - 1.0));
		if(tmp == 0)
			sift--;
		else
			sift += tmp/2;
	}
	/*
	cout << "Size (" << proc_image.cols << " * " << proc_image.rows << ")" << endl;
	cout << "interval:" << interval << endl;
	cout << "scale:" << scale << endl;
	cout << "sample_col_num:" << sample_col_num << endl;
	cout << "sample_row_num:" << sample_row_num << endl;
	cout << "odd_cols:" << odd_cols << endl;
	cout << "odd_rows:" << odd_rows << endl;
	cout << "sift:" << sift << endl;
	*/

	/*+ パラメータの算出 ++++++++++++++++*/
	// 1. キーポイントの抽出
	while(true)
	{
		cv::DenseFeatureDetector detector(floor(scale), 1, 0.1f, floor(interval), floor(sift), true, false);
		detector.detect(proc_image, keypoints);

		// 指定した特徴量になるまでフィードバックループ
		if(keypoints.size() < feature_num)
		{
			sift--;
			if(sift < 0)
				cerr << "計算式がおかしい" << endl;
		}
		else if(keypoints.size() > feature_num)
		{
			sift++;
			if(sift < 0)
				cerr << "計算式がおかしい" << endl;
		}
		else
			break;

	}

	// 2. SIFT記述子の抽出
	extractor.compute(proc_image, keypoints, descriptors);

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
		// x座標，y座標はスケーリングパラメータに応じて変化させる
		fout << (int)((itk->pt.x) / scale_x) << "\t";  // x座標
		fout << (int)((itk->pt.y) / scale_y) << "\t";  // y座標
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

	// 特徴量を書き込むためのリファレンス画像のコピーを作成
	cv::Mat tmp_image = ref_image.clone();
	if(resize == true)	// リサイズして特徴量を求めた場合は，リファレンス画像もリサイズする
		resize_image(tmp_image);

	// 特徴点を画像に書き出す．
	std::vector<cv::KeyPoint>::iterator itk;
	int keypoint_index = 0;
	for (itk = keypoints.begin(); itk != keypoints.end(); ++itk, keypoint_index++)
	{
		cv::circle(tmp_image, itk->pt, itk->size, cv::Scalar(0, 255, 255), 1, CV_AA);
		cv::circle(tmp_image, itk->pt, 1, cv::Scalar(0, 255, 0), -1);
	}

	string save = file_name;
	save.append("_sift.png");

	// Write image file.
	cv::imwrite(save, tmp_image);
}

void SIFTExtractor::resize_image(cv::Mat &image)
{
	cv::Mat tmp;

	// 画像の最も長い辺（横or縦）に合わせて正方形の画像を作成
	if(image.cols > image.rows)
	{
		// スケーリングパラメータの設定
		scale_x = 1.0;
		scale_y = image.cols / (double)image.rows;

		// image.cols ×　image.cols　サイズの画像を作成
		tmp = cv::Mat::zeros(image.cols, image.cols, CV_8UC1);
	}
	else
	{
		// スケーリングパラメータの設定
		scale_x = image.rows / (double)image.cols;
		scale_y = 1.0;

		// image.rows ×　image.rows　サイズの画像を作成
		tmp = cv::Mat::zeros(image.rows, image.rows, CV_8UC1);
	}

	// リサイズ
	cv::resize(image, tmp, tmp.size(), 0, 0);
	image = tmp.clone(); // リサイズ後のイメージをimageにコピー
}

#endif //_SIFTEXTRACTOR_HPP_
