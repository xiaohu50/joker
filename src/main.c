#include<stdio.h>
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"

CvMat* singleChannel( IplImage* img ){
       	CvMat* mat = cvCreateMat(img->height, img->width, CV_8UC1);
	cvCvtColor(img, mat, CV_BGR2GRAY);
	return mat;	
}

IplImage* mat2img( CvMat* mat ){
	IplImage* img = cvCreateImage(cvGetSize(mat), IPL_DEPTH_8U, 1);
	cvConvertScaleAbs(mat, img, 1, 0);
	cvGetImage(mat, img);
	return img;
}

CvMat* edgeDetect( CvMat* mat ){
	CvMat* mat1 = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	cvSobel(mat, mat1, 1, 0, 3);
	
	return mat1;
}


void dump2DMatU( CvMat* mat ){
	int i,j;
	for(i=0; i < mat->rows; i++){
		for(j=0; j < mat->cols; j++){
			printf("%d\t", CV_MAT_ELEM(*mat, unsigned, i, j));
		}
		printf("\n");
	}
}
void dump2DMatS( CvMat* mat ){
	int i,j;
	for(i=0; i < mat->rows; i++){
		for(j=0; j < mat->cols; j++){
			printf("%d\t", CV_MAT_ELEM(*mat, int, i, j));
		}
		printf("\n");
	}
}
void dump2DMatS5( CvMat* mat ){
	int i,j;
	for(i=0; i < 5; i++){
		for(j=0; j < 5; j++){
			printf("%d\t", CV_MAT_ELEM(*mat, int, i, j));
		}
		printf("\n");
	}
}
void dumpImage5( IplImage* img ){
	int i,j;
	CvScalar sc;
	for(i=0; i < 5; i++){
		for(j=0; j < 5; j++){
			sc = cvGet2D(img, i, j);
			printf("%d,%d,%d,%d\t", sc.val[0],sc.val[1],sc.val[2],sc.val[3]);
		}
		printf("\n");
	}
}
void dumpImage( IplImage* img ){
	int i,j;
	CvScalar sc;
	for(i=0; i < img->height; i++){
		for(j=0; j < img->width; j++){
			sc = cvGet2D(img, i, j);
			printf("%d,%d,%d,%d\t", sc.val[0],sc.val[1],sc.val[2],sc.val[3]);
		}
		printf("\n");
	}
}
void showHeadImage( IplImage* img, int count ){
	int i,j;
	uchar* ptr;
	for(i=0; i< count; i++){
		uchar* ptr = (uchar*)(img->imageData + i*img->widthStep);
		for(j=0; j< count; j++){
			printf("%d\t", *(ptr++));
		}
		printf("\n");
	}
	printf("\n");
}
int main( int argc, char** argv ){
	IplImage* img_tmp;
	IplImage* img0 = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	showHeadImage(img0, 5);
	cvNamedWindow("Picture-in", CV_WINDOW_AUTOSIZE);
	cvShowImage("Picture-in", img0);

	/*
	 * change the original picture into a single channel matrix
	 * */
	CvMat* mat1 = singleChannel(img0);
	img_tmp = mat2img(mat1);
	//dumpImage(img_tmp);
	showHeadImage(img_tmp, 5);
	cvNamedWindow("gray", CV_WINDOW_AUTOSIZE);
	cvShowImage("gray", img_tmp);
	cvReleaseImage(&img_tmp);


	/*
	 * edge detective
	 * */
	CvMat* mat2 = edgeDetect(mat1);
	dump2DMatS5(mat1);
	dump2DMatS5(mat2);
	img_tmp = mat2img(mat2);
	//dumpImage(img_tmp);
	showHeadImage(img_tmp, 5);
	cvNamedWindow("edge-detect", CV_WINDOW_AUTOSIZE);
	cvShowImage("edge-detect", img_tmp);
	cvReleaseImage(&img_tmp);
	


	/*
	 * destroy the windows, ending!!!!!!!!!!!!	
	 * */
	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvDestroyWindow("gray");
	cvDestroyWindow("edge-detect");
	return 0;
}
