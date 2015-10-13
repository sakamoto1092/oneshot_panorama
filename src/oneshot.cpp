/*
 * oneshot.cpp
 *
 *  Created on: 2015/09/29
 *      Author: k-sakamoto
 */
#include <opencv2/opencv.hpp>
//#include <opencv2/stitching.hpp>
#include<opencv2/stitching.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sstream>
#include <string>
#include <iostream>
#include<iomanip>
#include <vector>
#include <fstream>
#include"3dms-func.h"

#define PANO_W 6000
#define PANO_H 3000

using namespace std;
using namespace cv;


int main(int argc, char **argv){
	Mat panorama;
	Mat target;
	Mat mask;
	Mat pano_black;
	Mat white_img;
	Mat image;
	Mat homography;
	Mat transform_image, transform_image2;

	String str_pano;
	String str_log;
	String str_target;
	String tag;

	Mat a_tmp;

	if(argc != 6 ){
		cerr << "USAGE : " << argv[0] << " panorama_img" << " mask"<<" xml" << " target_img" << " frame_num" << endl;
		return -1;
	}

	str_pano = argv[1]; // パノラマ画像のパス
	str_log = argv[3]; // 射影行列が記録されているxmlファイルのパス
	str_target  = argv[4]; //合成フレームのパス
	tag = "homo_" + String(argv[5]); // xml内での射影行列のタグ

	//パノラマ画像をオープン
	panorama = imread(str_pano);
	if(panorama.empty()){
		cerr << "cannot open panorama image : " << str_pano << endl;
		//return -1;
		panorama = cv::Mat::zeros(Size(PANO_W, PANO_H), CV_8UC3);
	}
	cout << "read panoram"<<endl;
	// 射影行列のxmlをオープンして読みだす
	FileStorage cvfs_homo(str_log, CV_STORAGE_READ);
	if (!cvfs_homo.isOpened()) {
		cout << "cannt open internal parameta file " << str_log << endl;
		return -1;
	}

	cout << "<read homography>" << endl << "tag :" << tag <<endl;
	FileNode node_param(cvfs_homo.fs, NULL);
	read(node_param[tag], a_tmp);
	if(a_tmp.empty()){
		cerr << "cannot read homography : " << tag << endl;
		return -1;
	}
	cout << "<homography>" << endl << a_tmp <<endl;
	homography = a_tmp.clone();
	a_tmp.release();

	// 合成フレームをオープン
	image = imread(str_target);
	if(image.empty()){
		cerr << "cannot open target image :" << str_target <<endl;
		return -1;
	}

	pano_black = Mat(PANO_H, PANO_W, CV_8U, cv::Scalar(0));
	white_img = Mat(image.rows, image.cols, CV_8U, cv::Scalar(255));
	mask = Mat(PANO_H, PANO_W, CV_8U, cv::Scalar(0));
	transform_image2 = panorama.clone();

	// 先頭フレームをパノラマ平面へ投影
	warpPerspective(image, transform_image, homography, Size(PANO_W, PANO_H));
	warpPerspective(white_img, pano_black, homography, Size(PANO_W, PANO_H));



	make_pano(transform_image, transform_image2, mask, pano_black);

	Mat result_png = Mat(PANO_H, PANO_W,CV_8UC4);
	vector<Mat> mv;
	split(transform_image2,mv);
	mask = imread(argv[2],CV_LOAD_IMAGE_GRAYSCALE);
	mv.push_back(mask.clone());

	merge(mv,result_png);
	String name = "pano_movie"+String(argv[5]);
	imwrite(name+".jpg",transform_image2);
	imwrite(name+".png",result_png);

	namedWindow("result", CV_WINDOW_FREERATIO | CV_WINDOW_NORMAL);
	imshow("result",transform_image2);
	//waitKey(0);

	return 0;
}
