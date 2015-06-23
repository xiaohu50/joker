#include<stdio.h>
#include<strings.h>

#ifndef __JOKE_H__
#define __JOKE_H__
#include "joke.h"
#endif



JkMatList* jkCharLayer(IplImage* img){
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

int main( int argc, char** argv ){
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	IplImage* layer_color;
	//CvMat* layer = cvCreateMat(src->height, src->width, CV_8UC1);
	//CvMat* mask = cvCreateMat(src->height, src->width, CV_8UC1);
	char* name = new char[100];
	strcpy(name, argv[2]);
#ifdef DEBUG
	jkShowImageDetail(src);
	cvNamedWindow("Picture-in", 0);
	cvShowImage("Picture-in", src);
#endif

	JkMatList* mask_list = jkCharLayer(src);
	int index=0;
	char* index_c=new char[10];
	for(JkMatList* node = mask_list; node!=NULL; node = node->next){
		if(node->has_char){
			layer_color = jkMerge(src, node->mat);

			sprintf(index_c, "_jk%d.jpg", index);
			strcat(name, index_c);
			cvSaveImage(name, layer_color);
			strcpy(name, argv[2]);

			index++;
		}
	}
	

#ifdef DEBUG
	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvReleaseImage(&src);
#endif
}
