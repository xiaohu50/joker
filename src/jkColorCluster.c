#include<stdio.h>
#include<strings.h>
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "joke.h"

void jkReleaseArrList(JkArrList* list){
}

void jkReleaseBox2DList(JkBox2DList* list){
}

void jkBox2DDetail(CvBox2D box){
	printf("center:%f, %f\n",box.center.x, box.center.y);	
	printf("size: width:%f, height:%f\n", box.size.width, box.size.height);
	printf("angle:%f\n", box.angle);
}

void jkShowBox2D(CvArr* img, CvBox2D box){
	jkBox2DDetail(box);
	CvPoint2D32f boxPointsf[4];
	cvBoxPoints(box, boxPointsf);
	CvPoint points[4];
	for(int i=0; i<4; i++){
		points[i].x = boxPointsf[i].x;
		points[i].y = boxPointsf[i].y;
		printf("boxPoint[%d]=(x,y)=%d,%d\n",i,points[i].x,points[i].y);
	}
	cvLine(img, points[0], points[1], CVX_RED, 2);
	cvLine(img, points[1], points[2], CVX_BLUE, 2);
	cvLine(img, points[2], points[3], CVX_GREEN, 2);
	cvLine(img, points[3], points[0], CVX_WHITE, 2);

}

bool JkBoxSizeSuitable(CvBox2D* box, const IplImage* img){
	float img_min = MIN(img->width, img->height);
	float box_min = MIN(box->size.width, box->size.height);
	if(box_min>10 && box_min>img_min/10){
		return true;
	}

	return false;
}

void jkInsertPoint2BoxList(JkBox2DList* box_list, CvPoint p){
	float w,h;
	float cx,cy;
	float sinTheta, cosTheta;
	float x = p.x;
	float y = p.y;
			printf("point(x,y)=%f,%f\n",x,y);
	CvBox2D* box;
	for(JkBox2DList* box_list_p=box_list; box_list_p!=NULL; box_list_p = box_list_p->next){
		if(box_list_p->box){
			box = box_list_p->box;
			sinTheta = box_list_p->sinTheta;
			cosTheta = box_list_p->cosTheta;
			cx = box->center.x;
			cy = box->center.y;
			w = (cx-x)*sinTheta - (cy-y)*cosTheta;
			h = (cx-x)*cosTheta + (cy-y)*sinTheta;

			w = 2.0f - 4.0f*w/(box->size.width);
			h = 2.0f - 4.0f*h/(box->size.height);
			if(w>=0 && w<=4 && h>=0 && h<=4){
#ifdef DEBUG
				printf("*********************box corner**********************\n");
				jkBox2DDetail(*box);
				printf("w,h,4h+w:%f,%f,%d\n",w,h,4*(int)h+(int)w);
				printf("*****************************************************\n");
#endif
				box_list_p->corners[4*(int)h+(int)w]+=1;
			}
		}
	}
}

bool jkBoxCornerFlat(JkBox2DList* node){
	bool is_flat = true;
	int* s1 = node->corners;
	int* s2 = new int[9];
	int* s3 = new int[4];

	double s1a = 0, s2a = 0, s3a = 0;
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			s2[3*i+j] = s1[4*i+j] + s1[4*i+j+1] 
				+ s1[4*(i+1)+j] + s1[4*(i+1)+j+1];
			s2a += s2[3*i+j];
		}
	}
	s2a = s2a / 9.0f;

	for(int i=0; i<2; i++){
		for(int j=0; j<2; j++){
			s3[2*i+j] = s1[4*i+j] + s1[4*i+j+1] + s1[4*i+j+2]
				+ s1[4*(i+1)+j] + s1[4*(i+1)+j+1] + s1[4*(i+1)+j+2]
				+ s1[4*(i+2)+j] + s1[4*(i+2)+j+1] + s1[4*(i+2)+j+2];
			s3a += s3[2*i+j];
		}
	}
	s3a = s3a / 4.0f;

	for(int i=0; i<16; i++){
		s1a += s1[i];
	}
	s1a = s1a / 16.0f;

	if(s1a == 0 || s2a == 0 || s3a == 0)return false;

	double sd1 = 0, sd2 = 0, sd3 = 0;
	for(int i=0; i<16; i++){
		printf("s1[%d]=%d\n",i,s1[i]);
		sd1 += (s1[i]-s1a)*(s1[i]-s1a);
	}
	sd1 = sqrt(sd1/16);
	
	for(int i=0; i<9; i++){
		printf("s2[%d]=%d\n",i,s2[i]);
		sd2 += (s2[i]-s2a)*(s2[i]-s2a);
	}
	sd2 = sqrt(sd2/9);

	for(int i=0; i<4; i++){
		printf("s3[%d]=%d\n",i,s3[i]);
		sd3 += (s3[i]-s3a)*(s3[i]-s3a);
	}
	sd3 = sqrt(sd3/9);

	printf("sd1,s1a=%f,%f,%f\n",sd1,s1a,sd1/s1a);
	printf("sd2,s2a=%f,%f,%f\n",sd2,s2a,sd2/s2a);
	printf("sd3,s3a=%f,%f,%f\n",sd3,s3a,sd3/s3a);
	double score = (sd1/s1a) + (sd2/s2a) + (sd3/s3a);
	if(score>10){
		is_flat = false;
		printf("not flat, score:%f\n",score);
	}else{
		is_flat = true;
		printf("flat, score:%f\n",score);
	}

	delete[] s2;
	delete[] s3;


	return is_flat;
	
}
bool jkCharJudgeBox(JkBox2DList* box_list, IplImage* img){
	bool has_char = false;
	for(JkBox2DList* node = box_list; node!=NULL; node = node->next){
		CvBox2D* boxp = node->box;
		if(!JkBoxSizeSuitable(boxp, img)){
			free(boxp);
			node->box = NULL;
		}else{
			int* corners = (int*)malloc(sizeof(int)*16);
			bzero(corners, sizeof(int)*16);
			node->corners = corners;
			node->sinTheta = sin(boxp->angle * CV_PI /180);
			node->cosTheta = cos(boxp->angle * CV_PI /180);
		}
	}

	IplImage* img_8uc1 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	IplImage* eigImage = cvCreateImage(cvGetSize(img), IPL_DEPTH_32F, 1);
	IplImage* tempImage = cvCloneImage(eigImage);
	const int MAX_CORNER_COUNT=1000;
	CvPoint2D32f* corners = new CvPoint2D32f[MAX_CORNER_COUNT];
	int corner_count = MAX_CORNER_COUNT;
	cvCvtColor(img, img_8uc1, CV_RGB2GRAY);

	cvGoodFeaturesToTrack(
			img_8uc1,
			eigImage,
			tempImage,
			corners,
			&corner_count,
			0.01, //quality_level
			3, //min_distance
			NULL, //mask
			3, //block_size
			0, //use_harris
			0.4 //k
			);

	for(int i=0; i<corner_count; i++){
		printf("corner_count=%d,i=%d\n",corner_count,i);
		CvPoint cornerPoint = cvPoint((int)corners[i].x, (int)corners[i].y);
		cvCircle(img, cornerPoint, 1, CVX_RED, 1, 8);	
		jkInsertPoint2BoxList(box_list, cornerPoint);
	}

	for(JkBox2DList* box_list_p=box_list; box_list_p!=NULL; box_list_p = box_list_p->next){
		if(box_list_p->box){
			if(jkBoxCornerFlat(box_list_p)){
				jkShowBox2D(img, *(box_list_p->box));
				has_char = true;
			}
		}
	}

	cvNamedWindow("mask layer with corner", 0);
	cvShowImage("mask layer with corner", img);

	cvWaitKey(0);
	cvDestroyWindow("mask layer with corner");

	cvReleaseImage(&img_8uc1);
	cvReleaseImage(&eigImage);
	cvReleaseImage(&tempImage);
}

bool jkCharJudge(CvMat* mat, IplImage* img){
	CvMat* mat_tmp = cvCloneMat(mat);
	cvDilate(mat_tmp, mat_tmp, NULL, 2);
	cvErode(mat_tmp, mat_tmp, NULL, 3);
	cvDilate(mat_tmp, mat_tmp, NULL, 3);

	cvNamedWindow("mask use to contours", 0);
	cvShowImage("mask use to contours", mat_tmp);

	cvWaitKey(0);
	cvDestroyWindow("mask use to contours");


	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* first_contour = NULL;
	CvMat* mat_tmp2 = cvCloneMat(mat_tmp);
	int Nc = cvFindContours(mat_tmp2, storage, &first_contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	cvReleaseMat(&mat_tmp2);

	JkBox2DList* box_list=NULL;
	JkBox2DList* box_list_tail=NULL;
	for(CvSeq* c = first_contour; c!=NULL; c=c->h_next){
		CvBox2D box = cvMinAreaRect2(c);
		JkBox2DList* node = (JkBox2DList*)malloc(sizeof(JkBox2DList));
		CvBox2D* boxp = (CvBox2D*)malloc(sizeof(CvBox2D));
		memcpy(boxp, &box, sizeof(CvBox2D));
		node->box = boxp;
		node->corners = NULL;
		cvBoxPoints(box, node->boxPointsf);
		node->next = NULL;

		if(box_list){
			box_list_tail->next = node;
			box_list_tail = node;
		}else{
			box_list = node;
			box_list_tail = box_list;
		}
	}
	bool hasChar = jkCharJudgeBox(box_list, img);
	jkReleaseBox2DList(box_list);
	cvReleaseMat(&mat_tmp);

	return hasChar;
}

/* *
 *
 * img should be 3 channels, 8 depth
 * mask should be 1 channel, 8 depth
 *
 * */
IplImage* jkGetLayerFromMask(IplImage* img, CvMat* mask){
	if(img->nChannels != 3)return NULL;
	if(img->depth != 8)return NULL;

	IplImage* layer = cvCloneImage(img);
	for(int i=0; i<img->height; i++){
		uchar* ptr_layer = (uchar*)(layer->imageData + i * img->widthStep);
		uchar* ptr_mask = mask->data.ptr + (i+1) * mask->step;
		for(int j=0; j<img->width; j++){
			if(ptr_mask[j+1]==0){
				ptr_layer[3*j]=0;
				ptr_layer[3*j+1]=0;
				ptr_layer[3*j+2]=0;
			}	
		}
	}

	return layer;
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
	
	IplImage* img_layer;
	for(JkArrList* node = mask_list; node != NULL; node = node->next){
		img_layer = jkGetLayerFromMask(img_tmp, (CvMat*)(node->arr));
		cvNamedWindow("img_layer", 0);
		cvShowImage("img_layer", img_layer);
		if(jkCharJudge((CvMat*)(node->arr), img_layer)){
			cvOr(mask, node->arr, mask, NULL);
		}
		cvReleaseImage(&img_layer);
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
