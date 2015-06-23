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
	return img;
}

CvMat* edgeDetect1( CvMat* mat ){
	CvMat* mat1 = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	CvMat* mat2 = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	CvMat* mat3 = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	cvSobel(mat, mat1, 1, 0, 3);
	cvSobel(mat, mat2, 0, 1, 3);
	cvSobel(mat, mat3, 1, 1, 3);

	CvMat* mat_tmp = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	cvMax(mat1, mat2, mat_tmp);
	cvMax(mat_tmp, mat3, mat1);
	cvReleaseMat(&mat_tmp);
	cvReleaseMat(&mat2);
	
	return mat1;
}

CvMat* edgeDetect2( CvMat* mat ){
	CvMat* mat1 = cvCreateMat(mat->rows, mat->cols, CV_16SC1);
	cvSobel(mat, mat1, 1, 0, 3);

	
	return mat1;
}
void showConst(){
	printf("CV_8UC1\t%d\n", CV_8UC1);
	printf("CV_8UC2\t%d\n", CV_8UC2);
	printf("CV_8UC3\t%d\n", CV_8UC3);

	printf("CV_16SC1\t%d\n", CV_16SC1);
	printf("CV_16SC2\t%d\n", CV_16SC2);
	printf("CV_16SC3\t%d\n", CV_16SC3);
}
void dump2DMat8U( CvMat* mat, int count ){
	int i,j;
	int mt = cvGetElemType(mat);
	if(mt == CV_8UC1){
		printf("MATRIX\n element type: CV_8UC1(%d)\n",mt);
	}else{
		printf("MATRIX\n element type: unknown(%d)\n",mt);
	}
	for(i=0; i < count; i++){
		for(j=0; j < count; j++){
			printf("%d\t", CV_MAT_ELEM(*mat, uchar, i, j));
		}
		printf("\n");
	}
	printf("\n");
}
void dump2DMat16S( CvMat* mat, int count ){
	int i,j;
	CvScalar  sc;
	int mt = cvGetElemType(mat);
	if(mt == CV_16SC1){
		printf("MATRIX\n element type: CV_16SC1(%d)\n",mt);
	}else{
		printf("MATRIX\n element type: unknown(%d)\n",mt);
	}
	for(i=0; i < count; i++){
		for(j=0; j < count; j++){
			sc = cvGet2D(mat, i, j);
			//printf("%0.2f,%0.2f,%0.2f,%0.2f\t", sc.val[0], sc.val[1], sc.val[2], sc.val[3]);
			printf("%0.2f\t", sc.val[0]);
		}
		printf("\n");
	}
	printf("\n");
}
void showHeadImage( IplImage* img, int count ){
	int i,j;
	uchar* ptr;
	CvScalar  sc;
	printf("IMAGE\n");
	if(img->dataOrder == IPL_ORIGIN_TL){
		printf("dataOrder: IPL_ORIGIN_TL\n");
	}else if(img->dataOrder == IPL_ORIGIN_BL){
		printf("dataOrder: IPL_ORIGIN_BL\n");
	}
	for(i=0; i< count; i++){
		uchar* ptr = (uchar*)(img->imageData + i*img->widthStep);
		for(j=0; j< count; j++){
			sc = cvGet2D(img, i, j);
			printf("%d|%0.1f,%0.1f,%0.1f,%0.1f\t", *(ptr++), sc.val[0], sc.val[1], sc.val[2], sc.val[3]);
		}
		printf("\n");
	}
	printf("\n");
}
int main( int argc, char** argv ){
	IplImage* img_tmp;
	IplImage* img0 = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	showConst();
	showHeadImage(img0, 5);
	cvNamedWindow("Picture-in", CV_WINDOW_AUTOSIZE);
	cvShowImage("Picture-in", img0);

	/*
	 * change the original picture into a single channel matrix
	 * */
	CvMat* mat1 = singleChannel(img0);
	dump2DMat8U(mat1, 5);
	img_tmp = mat2img(mat1);
	showHeadImage(img_tmp, 5);
	cvNamedWindow("gray", CV_WINDOW_AUTOSIZE);
	cvShowImage("gray", img_tmp);
	cvReleaseImage(&img_tmp);


	/*
	 * edge detective
	 * */
	CvMat* mat2 = edgeDetect1(mat1);
	dump2DMat16S(mat2, 5);
	img_tmp = mat2img(mat2);
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
