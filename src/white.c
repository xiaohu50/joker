#include<stdio.h>
#include<strings.h>

#ifndef __JOKE_H__
#define __JOKE_H__
#include "joke.h"
#endif

int main( int argc, char** argv ){
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	IplImage* src_gray = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* src_white = cvCreateImage(cvGetSize(src), 8, 1);
	cvCvtColor(src, src_gray, CV_RGB2GRAY);
	cvThreshold(src_gray, src_white, 10, 255, CV_THRESH_BINARY_INV); 

	char* name = new char[100];
	strcpy(name, argv[2]);
	cvSaveImage(name, src_white);
}
