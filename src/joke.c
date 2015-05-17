#include<stdio.h>

#ifndef __JOKE_H__
#define __JOKE_H__
#include "joke.h"
#endif

IplConvKernel* KERNEL_3R1C = cvCreateStructuringElementEx(3,1,1,0,CV_SHAPE_RECT);
IplConvKernel* KERNEL_9R1C = cvCreateStructuringElementEx(9,1,4,0,CV_SHAPE_RECT);
IplConvKernel* KERNEL_1R3C = cvCreateStructuringElementEx(1,3,0,1,CV_SHAPE_RECT);

JkFeatureLayer* jkFeaturePoint(IplImage* img){
	JkFeatureLayer* layer = (JkFeatureLayer*)malloc(sizeof(JkFeatureLayer));
	layer->mat = cvCreateMat(img->height, img->width, CV_8UC1);
	cvSetZero(layer->mat);

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

	layer->corners = new CvPoint2D32f[corner_count];
	memcpy(layer->corners, corners, sizeof(CvPoint2D32f)*corner_count);
	layer->corner_count = corner_count;
	for(int i=0; i<corner_count; i++){
		*((uint8_t*)CV_MAT_ELEM_PTR(*layer->mat, (int)corners[i].y, (int)corners[i].x)) = 255;
	}
#ifdef DEBUG
	cvNamedWindow("mask layer with corner", 0);
	cvShowImage("mask layer with corner", layer->mat);

	cvWaitKey(0);
	cvDestroyWindow("mask layer with corner");
#endif

	cvReleaseImage(&img_8uc1);
	cvReleaseImage(&eigImage);
	cvReleaseImage(&tempImage);
	delete [] corners;

	return layer;
}

int jkScalarBigger(const CvScalar a, const CvScalar b){
	if(a.val[0]<b.val[0] || a.val[1]<b.val[1] || a.val[2]<b.val[2] || a.val[3]<b.val[3])return -1;
	if(a.val[0]==b.val[0] && a.val[1]==b.val[1] && a.val[2]==b.val[2] && a.val[3]==b.val[3])return 0;
	return 1;
}

int jkScalarDiff(const CvScalar a, const CvScalar b, CvScalar* c){
	c->val[0]=a.val[0]-b.val[0];
	c->val[1]=a.val[1]-b.val[1];
	c->val[2]=a.val[2]-b.val[2];
	c->val[3]=a.val[3]-b.val[3];
	return 0;
}

CvMat* jkMinus1(CvMat* mat){
	CvMat* mat2 = cvCreateMat(mat->height-2, mat->width-2, CV_8UC1);
	for(int row = 1; row < mat->rows-1; row++){
		uint8_t* ptr = (uint8_t*)(mat->data.ptr + row * mat->step);
		uint8_t* ptr2 = (uint8_t*)(mat2->data.ptr + (row-1) * mat2->step);
		for(int col = 1; col < mat->cols-1; col++){
			*ptr2++ = *ptr++;
		}
	}
	return mat2;
}
JkMatList* jkGetColorLayer(IplImage* img){
	JkMatList* mask_list = NULL;
	JkMatList* mask_list_tail = NULL;
	int rows = img->height;	
	int cols = img->width;
	
	CvMat* mask = cvCreateMat(rows+2, cols+2, CV_8UC1);
	CvMat* mask_layer = cvCreateMat(rows+2, cols+2, CV_8UC1);
	CvMat* mask_zero = cvCreateMat(rows+2, cols+2, CV_8UC1);

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
			CvScalar seed = cvGet2D(img, i, j);
			unsigned char checkPoint = CV_MAT_ELEM(*mask, unsigned char, i+1, j+1);
			if(checkPoint == 0){
				cvSetZero(mask_layer);
				printf("i=%d,j=%d\n",i,j);
				for(int x=0; x<rows; x++){
					for(int y=0; y<cols; y++){
						unsigned char checkPoint_inner = CV_MAT_ELEM(*mask_layer, unsigned char, x+1, y+1);
						if(checkPoint_inner == 0){
							//printf("x=%d,y=%d\n",x,y);
							CvScalar seed_inner = cvGet2D(img, x, y);
							CvScalar diff1, diff2;
							jkScalarDiff(seed_inner, seed, &diff1);
							jkScalarDiff(seed, seed_inner, &diff2);
							if(jkScalarBigger(loDiff_color, diff2)>=0 
									&& jkScalarBigger(upDiff_color, diff1)>=0){
								cvSetZero(mask_zero);
								CvPoint seedPoint = cvPoint(y, x);
								cvFloodFill(img, 
										seedPoint, 
										newVal, 
										loDiff_color, 
										upDiff_color, 
										NULL, 
										flags_color, 
										mask_zero);
								cvFloodFill(img, 
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

				JkMatList* node = (JkMatList*)malloc(sizeof(JkMatList));
				node->mat = jkMinus1(mask_layer);
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

	cvReleaseMat(&mask);
	cvReleaseMat(&mask_layer);
	cvReleaseMat(&mask_zero);

	return mask_list;
}

JkBoxList* jkGetBox(CvMat* mask){
	CvMat* mat_tmp = cvCloneMat(mask);
	cvDilate(mat_tmp, mat_tmp, NULL, 2);
	cvErode(mat_tmp, mat_tmp, NULL, 3);
	cvDilate(mat_tmp, mat_tmp, NULL, 3);
	
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* first_contour = NULL;
	CvMat* mat_tmp2 = cvCloneMat(mat_tmp);
	int Nc = cvFindContours(mat_tmp2, storage, &first_contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	cvReleaseMat(&mat_tmp2);

	JkBoxList* box_list=NULL;
	JkBoxList* box_list_tail=NULL;
	for(CvSeq* c = first_contour; c!=NULL; c=c->h_next){
		CvBox2D box = cvMinAreaRect2(c);
		JkBoxList* node = (JkBoxList*)malloc(sizeof(JkBoxList));
		node->box = box;
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

	cvReleaseMat(&mat_tmp);
	return box_list;
}

int jkInsertPoint2BoxList(JkBoxList* box_list, CvPoint p){
	float w,h;
	float cx,cy;
	float sinTheta, cosTheta;
	float x = p.x;
	float y = p.y;
	int position=0;
	int index=0;
	for(JkBoxList* node=box_list; node!=NULL; node = node->next, index++){
		if(node->available){
			sinTheta = node->sinTheta;
			cosTheta = node->cosTheta;
			cx = node->box.center.x;
			cy = node->box.center.y;
			w = (cx-x)*cosTheta + (cy-y)*sinTheta;
			h = (cx-x)*sinTheta - (cy-y)*cosTheta;

			w = 2.0f - 4.0f*w/(node->box.size.width);
			h = 2.0f - 4.0f*h/(node->box.size.height);
			if(w>=0 && w<=4 && h>=0 && h<=4){
				position = index;
				node->corners[4*(int)h+(int)w]+=1;
			}
		}
	}
	return position;
}

IplImage* jkGetLayerFromMask(IplImage* img, CvMat* mask){
	if(img->nChannels != 3)return NULL;
	if(img->depth != 8)return NULL;

	IplImage* layer = cvCloneImage(img);
	for(int i=0; i<img->height; i++){
		uchar* ptr_layer = (uchar*)(layer->imageData + i * img->widthStep);
		uchar* ptr_mask = mask->data.ptr + i * mask->step;
		for(int j=0; j<img->width; j++){
			if(ptr_mask[j]==0){
				ptr_layer[3*j]=0;
				ptr_layer[3*j+1]=0;
				ptr_layer[3*j+2]=0;
			}	
		}
	}

	return layer;
}

bool jkBoxCornerFlat(JkBoxList* node){
	bool is_flat = true;
	int* s1 = node->corners;
	int* s2 = new int[9];
	int* s3 = new int[4];
	int* s12h = new int[12];
	int* s12v = new int[12];

	double s1a = 0, s2a = 0, s3a = 0, s12ha = 0, s12va = 0, s24a = 0;
	for(int i=0; i<16; i++){
		s1a += s1[i];
	}
	s1a = s1a / 16.0f;

	double density=0;
	density = (s1a*16) / (node->box.size.width * node->box.size.height);
	if(density<0.001){
		is_flat = false;
		printf("not flat, density:%f\n",density);
		return is_flat;
	}else{
		is_flat = true;
		printf("flat, density:%f\n",density);
	}

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


	for(int i=0; i<4; i++){
		for(int j=0; j<3; j++){
			s12h[3*i+j] = s1[4*i+j] + s1[4*i+j+1];
			s12ha += s12h[3*i+j];
		}
	}
	for(int i=0; i<3; i++){
		for(int j=0; j<4; j++){
			s12v[4*i+j] = s1[4*i+j] + s1[4*(i+1)+j];
			s12va += s12v[4*i+j];
		}
	}
	s24a = (s12ha + s12va) / 24.0f;

	if(s1a == 0 || s2a == 0 || s3a == 0 || s24a == 0)return false;

	double sd1 = 0, sd2 = 0, sd3 = 0, sd24 = 0;
	for(int i=0; i<16; i++){
		//printf("s1[%d]=%d\n",i,s1[i]);
		sd1 += (s1[i]-s1a)*(s1[i]-s1a);
	}
	sd1 = sqrt(sd1/16);
	
	for(int i=0; i<9; i++){
		//printf("s2[%d]=%d\n",i,s2[i]);
		sd2 += (s2[i]-s2a)*(s2[i]-s2a);
	}
	sd2 = sqrt(sd2/9);

	for(int i=0; i<4; i++){
		//printf("s3[%d]=%d\n",i,s3[i]);
		sd3 += (s3[i]-s3a)*(s3[i]-s3a);
	}
	sd3 = sqrt(sd3/9);

	for(int i=0; i<12; i++){
		//printf("s12h[%d]=%d\n",i,s12h[i]);
		sd24 += (s12h[i]-s24a)*(s12h[i]-s24a);
	}
	for(int i=0; i<12; i++){
		//printf("s12v[%d]=%d\n",i,s12v[i]);
		sd24 += (s12v[i]-s24a)*(s12v[i]-s24a);
	}
	sd24 = sqrt(sd24/24);

	printf("sd1,s1a=%f,%f,%f\n",sd1,s1a,sd1/s1a);
	printf("sd2,s2a=%f,%f,%f\n",sd2,s2a,sd2/s2a);
	printf("sd3,s3a=%f,%f,%f\n",sd3,s3a,sd3/s3a);
	printf("sd24,s24a=%f,%f,%f\n",sd24,s24a,sd24/s24a);

	double score = (sd1/s1a) + (sd2/s2a) + (sd3/s3a) + (sd24/s24a);
	if(score>10){
		is_flat = false;
		printf("not flat, score:%f\n",score);
	}else{
		is_flat = true;
		printf("flat, score:%f\n",score);
	}

	delete [] s2;
	delete [] s3;
	delete [] s12h;
	delete [] s12v;


	return is_flat;
}

bool jkJudgeLayer(IplImage* img, CvMat* mask, JkBoxList* box_list, JkFeatureLayer* feature_layer){
	bool has_char = false;
#ifdef DEBUG
	IplImage* img_layer = jkGetLayerFromMask(img, mask);
#endif
	
	CvPoint2D32f* corners = feature_layer->corners;
	int position;
	for(int i=0; i<feature_layer->corner_count; i++){
		CvPoint cornerPoint = cvPoint((int)corners[i].x, (int)corners[i].y);
		position = jkInsertPoint2BoxList(box_list, cornerPoint);
#ifdef DEBUG
		if(position == 0){
			cvCircle(img_layer, cornerPoint, 1, CVX_RED, 1, 8);	
		}else{
			cvCircle(img_layer, cornerPoint, 1, CV_RGB(position*position*10 % 255, (position*50) % 255, (position*13) % 255), 1, 8);	
		}
#endif
	}

	for(JkBoxList* node=box_list; node!=NULL; node = node->next){
		if(node->available){
			if(jkBoxCornerFlat(node)){
#ifdef DEBUG
				jkShowBox(img_layer, node->box);
#endif
				has_char = true;
			}
		}
	}
#ifdef DEBUG
	/*
	cvNamedWindow("mask layer with corner", 0);
	cvShowImage("mask layer with corner", img_layer);

	cvWaitKey(0);
	cvDestroyWindow("mask layer with corner");
	printf("come here!!!!!!!!!!!!!!!\n");
	*/
	cvReleaseImage(&img_layer);
#endif

	return has_char;
}

void jkShowImageDetail(IplImage* img){
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


bool jkBoxSizeSuitable(CvBox2D box, const IplImage* img){
	float img_min = MIN(img->width, img->height);
	float box_min = MIN(box.size.width, box.size.height);
	if(box_min>10 && box_min>img_min/10){
		return true;
	}

	return false;
}
void jkSizeFilterBoxList(IplImage* img, JkBoxList* box_list){
	for(JkBoxList* node = box_list; node!=NULL; node = node->next){
		CvBox2D box = node->box;
		if(!jkBoxSizeSuitable(box, img)){
			node->available = false;
		}else{
			node->available = true;
			int* corners = (int*)malloc(sizeof(int)*16);
			bzero(corners, sizeof(int)*16);
			node->corners = corners;
			node->sinTheta = sin(box.angle * CV_PI /180);
			node->cosTheta = cos(box.angle * CV_PI /180);
		}
	}
}
void jkShowBoxMsg(CvBox2D box){
	printf("center:%f, %f\n",box.center.x, box.center.y);	
	printf("size: width:%f, height:%f\n", box.size.width, box.size.height);
	printf("angle:%f\n", box.angle);
}
void jkShowBox(CvArr* img, CvBox2D box){
	jkShowBoxMsg(box);
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

void jkShowBoxList(IplImage* img, JkBoxList* box_list){
	IplImage* img_tmp = cvCloneImage(img);
	for(JkBoxList* node = box_list; node!=NULL; node = node->next){
		if(node->available){
			jkShowBox(img_tmp, node->box);
		}
	}
	cvNamedWindow("box list after sizefilter", 0);
	cvShowImage("box list after sizefilter", img_tmp);

	cvWaitKey(0);
	cvDestroyWindow("box list after sizefilter");
	cvReleaseImage(&img_tmp);
}

IplImage* jkMerge(IplImage* img, CvMat* mask_final){
	return jkGetLayerFromMask(img, mask_final);
}

void jkReleaseBoxList(JkBoxList** box_list){
	JkBoxList* next=NULL;
	for(JkBoxList* node = (*box_list); node!=NULL; node = next){
		delete [] node->corners;
		next = node->next;
		free(node);
	}
	*box_list = NULL;
}

void jkReleaseMatList(JkMatList** mat_list){
	JkMatList* next=NULL;
	for(JkMatList* node = (*mat_list); node!=NULL; node = next){
		cvReleaseMat(&(node->mat));
		next = node->next;
		free(node);
	}
	*mat_list = NULL;
}

void jkReleaseFeatureLayer(JkFeatureLayer** feature_layer){
	cvReleaseMat(&((*feature_layer)->mat));
	delete [] (*feature_layer)->corners;
	free(*feature_layer);
	*feature_layer = NULL;
}
