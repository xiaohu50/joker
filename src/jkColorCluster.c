#include<stdio.h>
#include<strings.h>

#ifndef __JOKE_H__
#define __JOKE_H__
#include "joke.h"
#endif



IplImage* jkCharLayer(IplImage* img){
	bool remain = false;
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
	CvMat* mask_final = cvCreateMat(img->height, img->width, CV_8UC1);
	cvSetZero(mask_final);
	JkMatList* mask_list = jkGetColorLayer(img);
	CvMat* mask;
	for(JkMatList* node = mask_list; node!=NULL; node = node->next){
		mask = node->mat;

		JkBoxList* box_list = jkGetBox(mask);//get box list for each color layer
		jkSizeFilterBoxList(img, box_list);
#ifdef DEBUG
		jkShowBoxList(img, box_list);
#endif

		bool has_char = jkJudgeLayer(img, mask, box_list, feature_layer);//judge whether the layer has characters
		if(has_char){
			cvOr(mask, mask_final, mask_final, NULL);
			remain = true;
		}
		jkReleaseBoxList(&box_list);
	}
	jkReleaseMatList(&mask_list);
	jkReleaseFeatureLayer(&feature_layer);

	if(remain){
		IplImage* img_result = jkMerge(img, mask_final);//change on 'img'
		cvReleaseMat(&mask_final);
		return img_result;
	}
	cvReleaseMat(&mask_final);

	cvReleaseImage(&img_tmp);
	return NULL;
}

#ifdef DEBUG
int main( int argc, char** argv ){
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	jkShowImageDetail(src);
	cvNamedWindow("Picture-in", 0);
	cvShowImage("Picture-in", src);

	IplImage* dst = jkCharLayer(src);
	if(dst != NULL){
		cvNamedWindow("result", 0);
		cvShowImage("result", dst);
		cvWaitKey(0);
		cvDestroyWindow("result");
		cvReleaseImage(&dst);
	}

	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvReleaseImage(&src);
}
#endif
