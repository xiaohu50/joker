#include<stdio.h>
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "joke.h"

#define CVX_WHITE  CV_RGB(255, 255, 255)
#define CVX_RED  CV_RGB(255, 0, 0)
#define CVX_BLUE  CV_RGB(0, 0, 255)

void jkReleaseArrList(JkArrList* list){
}
bool jkCharJudge(CvArr* mat){
	cvNamedWindow("each mask", 0);
	cvShowImage("each mask", mat);

	cvWaitKey(0);
	cvDestroyWindow("each mask");
}

IplImage* jkCharPick(IplImage* src){
	if(src->nChannels != 3)return NULL;
	IplImage* img_tmp = cvCreateImage(cvGetSize(src), 8, 3);
	cvPyrMeanShiftFiltering(
			src,
			img_tmp,
			2,
			10,
			2
			);
#ifdef DEBUG
	cvNamedWindow("meanshift", 0);
	cvShowImage("meanshift", img_tmp);
#endif

	JkArrList* mask_list = jkColorCluster(img_tmp);
	CvMat* mask = cvCreateMat(img_tmp->height+2, img_tmp->width+2, CV_8UC1);
	cvSetZero(mask);
	for(JkArrList* node = mask_list; node != NULL; node = node->next){
		if(jkCharJudge(node->arr)){
			cvOr(mask, node->arr, mask, NULL);
		}
	}
	jkReleaseArrList(mask_list);
	cvNamedWindow("mask final", 0);
	cvShowImage("mask final", mask);

	cvWaitKey(0);
	cvDestroyWindow("mask final");
	
	return img_tmp;
}

JkArrList* jkColorCluster(IplImage* src){
	JkArrList* mask_list = NULL;
	JkArrList* mask_list_tail = NULL;
	int rows = src->height;	
	int cols = src->width;
	
	CvMat* mask = cvCreateMat(rows+2, cols+2, CV_8UC1);
	CvMat* mask_zero = cvCreateMat(rows+2, cols+2, CV_8UC1);
	jkMatShowDetail(mask);

	CvScalar loDiff_color = CV_RGB(30, 30, 30);
	CvScalar upDiff_color = CV_RGB(30, 30, 30);
	CvScalar loDiff_connect = CV_RGB(10, 10, 10);
	CvScalar upDiff_connect = CV_RGB(10, 10, 10);
	CvScalar newVal = CV_RGB(255, 255, 255);
	int flags_color = 8
		| CV_FLOODFILL_MASK_ONLY		
		| CV_FLOODFILL_FIXED_RANGE
		| (255<<8);
	int flags_connect = 8
		| CV_FLOODFILL_MASK_ONLY		
		| (255<<8);

	cvSetZero(mask);
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			CvScalar seed = cvGet2D(src, i, j);
			unsigned char checkPoint = CV_MAT_ELEM(*mask, unsigned char, i+1, j+1);
			if(checkPoint == 0){
				CvMat* mask_layer = cvCreateMat(rows+2, cols+2, CV_8UC1);
				cvSetZero(mask_layer);
				printf("i=%d,j=%d\n",i,j);
				for(int x=0; x<rows; x++){
					for(int y=0; y<cols; y++){
						unsigned char checkPoint_inner = CV_MAT_ELEM(*mask_layer, unsigned char, x+1, y+1);
						if(checkPoint_inner == 0){
							//printf("x=%d,y=%d\n",x,y);
							CvScalar seed_inner = cvGet2D(src, x, y);
							CvScalar diff1, diff2;
							jkScalarDiff(seed_inner, seed, &diff1);
							jkScalarDiff(seed, seed_inner, &diff2);
							if(jkScalarBigger(loDiff_color, diff2)>=0 
									&& jkScalarBigger(upDiff_color, diff1)>=0){
								/*
								printf("x=%d,y=%d",x,y);
								jkShowScalar(seed_inner);
								jkShowScalar(seed);
								jkShowScalar(diff1);
								jkShowScalar(diff2);
								*/
								cvSetZero(mask_zero);
								CvPoint seedPoint = cvPoint(y, x);
								cvFloodFill(src, 
										seedPoint, 
										newVal, 
										loDiff_color, 
										upDiff_color, 
										NULL, 
										flags_color, 
										mask_zero);
								cvFloodFill(src, 
										seedPoint, 
										newVal, 
										loDiff_connect, 
										upDiff_connect, 
										NULL, 
										flags_connect, 
										mask_zero);
							
								cvOr(mask_layer, mask_zero, mask_layer);

							}
						}
					}
				}
				cvOr(mask, mask_layer, mask);

				JkArrList* node = (JkArrList*)malloc(sizeof(JkArrList));
				node->arr = mask_layer;
				node->next = NULL;

				if(mask_list){
					mask_list_tail->next = node;
					mask_list_tail = node;
				}else{
					mask_list = node;
					mask_list_tail = mask_list;
				}
				
			}
		}
	}

	return mask_list;
}

void showConst(){
	printf("CV_8UC1\t%d\n", CV_8UC1);
	printf("CV_8UC2\t%d\n", CV_8UC2);
	printf("CV_8UC3\t%d\n", CV_8UC3);

	printf("CV_16SC1\t%d\n", CV_16SC1);
	printf("CV_16SC2\t%d\n", CV_16SC2);
	printf("CV_16SC3\t%d\n", CV_16SC3);
}

void jkMatShowDetail(CvMat* mat){
	printf("****************mat*****************\n");
	printf("rows:%d\n",mat->rows);
	printf("cols:%d\n",mat->cols);
	printf("************************************\n");
}
void jkImgShowDetail(IplImage* img){
	printf("****************img*****************\n");
	printf("nsize:%d\n",img->nSize);
	printf("ID:%d\n",img->ID);
	if(img->origin==IPL_ORIGIN_TL){
		printf("origin:IPL_ORIGIN_TL\n");
	}else{
		printf("origin:IPL_ORIGIN_BL\n");
	}
	printf("channels:%d\n",img->nChannels);
	printf("depth:%d\n",img->depth);
	printf("width:%d\n",img->width);
	printf("height:%d\n",img->height);
	printf("************************************\n");
}
void jkShowScalar(CvScalar cs){
	printf("val=%f,%f,%f,%f\n",cs.val[0],cs.val[1],cs.val[2],cs.val[3]);
}
int jkScalarDiff(const CvScalar a, const CvScalar b, CvScalar* c){
	c->val[0]=a.val[0]-b.val[0];
	c->val[1]=a.val[1]-b.val[1];
	c->val[2]=a.val[2]-b.val[2];
	c->val[3]=a.val[3]-b.val[3];
	return 0;
}
int jkScalarConverse(CvScalar a, CvScalar* b){
	b->val[0] = 0-a.val[0];
	b->val[1] = 0-a.val[1];
	b->val[2] = 0-a.val[2];
	b->val[3] = 0-a.val[3];
	return 0;
}
int jkScalarSmaller(const CvScalar a, const CvScalar b){
	if(a.val[0]>b.val[0] || a.val[1]>b.val[1] || a.val[2]>b.val[2] || a.val[3]>b.val[3])return -1;
	if(a.val[0]==b.val[0] && a.val[1]==b.val[1] && a.val[2]==b.val[2] && a.val[3]==b.val[3])return 0;
	return 1;
}
int jkScalarBigger(const CvScalar a, const CvScalar b){
	if(a.val[0]<b.val[0] || a.val[1]<b.val[1] || a.val[2]<b.val[2] || a.val[3]<b.val[3])return -1;
	if(a.val[0]==b.val[0] && a.val[1]==b.val[1] && a.val[2]==b.val[2] && a.val[3]==b.val[3])return 0;
	return 1;
}

#ifdef DEBUG
int main( int argc, char** argv ){
	showConst();
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	jkImgShowDetail(src);
	cvNamedWindow("Picture-in", 0);
	cvShowImage("Picture-in", src);

	IplImage* dst = jkCharPick(src);
	cvNamedWindow("result", 0);
	cvShowImage("result", dst);

	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvDestroyWindow("result");
}
#endif
