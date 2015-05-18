#include<stdio.h>
#include<strings.h>

#ifndef __JOKE_H__
#define __JOKE_H__
#include "joke.h"
#endif



JkMatList* jkCharLayer(IplImage* img){
	/*
	IplImage* img_8uc1 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	IplImage* img_bi = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	cvCvtColor(img, img_8uc1, CV_RGB2GRAY);
	cvAdaptiveThreshold(
			img_8uc1,
			img_bi,
			255,
			CV_ADAPTIVE_THRESH_MEAN_C,
			CV_THRESH_BINARY_INV,
			3,
			5
			);
#ifdef DEBUG
	cvNamedWindow("binaryzation", 0);
	cvShowImage("binaryzation", img_bi);
#endif
	*/
	

	/********************************** filtering ************************************/
	IplImage* img_tmp = cvCreateImage(cvGetSize(img), 8, 3);
	cvPyrMeanShiftFiltering(
			img,
			img_tmp,
			2,
			20,
			2
			);
#ifdef DEBUG
	cvNamedWindow("meanshift", 0);
	cvShowImage("meanshift", img_tmp);
#endif
	img = img_tmp;


	/******************************get feature points*********************************/
	JkFeatureLayer* feature_layer = jkFeaturePoint(img);

	/****************************get different color layer****************************/
	JkMatList* mask_list = jkGetColorLayer(img);
	CvMat* mask;
	for(JkMatList* node = mask_list; node!=NULL; node = node->next){
		mask = node->mat;

		JkBoxList* box_list = jkGetBox(mask);//get box list for each color layer
		jkSizeFilterBoxList(img, box_list);
#ifdef DEBUG
		//jkShowBoxList(img, box_list);
#endif

		node->has_char = jkJudgeLayer(img, mask, box_list, feature_layer);//judge whether the layer has characters
		jkReleaseBoxList(&box_list);
	}
	jkReleaseFeatureLayer(&feature_layer);

	cvReleaseImage(&img_tmp);
	return mask_list;
}

#ifdef DEBUG
int main( int argc, char** argv ){
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	jkShowImageDetail(src);
	cvNamedWindow("Picture-in", 0);
	cvShowImage("Picture-in", src);

	JkMatList* mask_list = jkCharLayer(src);
	for(JkMatList* node = mask_list; node!=NULL; node = node->next){
		if(node->has_char){
			cvNamedWindow("haschar layer", 0);
			cvShowImage("haschar layer", node->mat);
			cvWaitKey(0);
			cvDestroyWindow("haschar layer");
		}
	}
	

	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvReleaseImage(&src);
}
#endif
