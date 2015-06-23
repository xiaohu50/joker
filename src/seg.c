#include<stdio.h>
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"

#define CVX_WHITE  CV_RGB(255, 255, 255)
#define CVX_RED  CV_RGB(255, 0, 0)
#define CVX_BLUE  CV_RGB(0, 0, 255)
#define DESCENDING_ORDER 0
#define ASCENDING_ORDER 1
int compareAscendingF(const void* a, const void* b){
	float x,y;
	x = (float)*((float*)a);
	y = (float)*((float*)b);
	if(x<y)return -1;
	if(x>y)return 1;
	return 0;
}
int compareDescendingF(const void* a, const void* b){
	float x,y;
	x = (float)*((float*)a);
	y = (float)*((float*)b);
	if(x>y)return -1;
	if(x<y)return 1;
	return 0;
}


/* *
 * hist: the histogram handle, only support CV_HIST_ARRAY
 * scale: the scale you want to keep
 * order: order the node in increasing order or not
 * */
int jkThreshHist2D(CvHistogram* hist, const double scale, const int order){
	if(hist == NULL || scale > 1 || scale < 0)return -1;
	if(!(order == DESCENDING_ORDER || order == ASCENDING_ORDER))return -1;
	int dims = hist->mat.dims;
	if(dims!=2)return -1;

#ifdef DEBUG
	printf("-----------------jkThreshHist message-----------------------\n");
	if(order == ASCENDING_ORDER){
		printf("type: ASCENDING_ORDER\n");
	}else{
		printf("type: DESCENDING_ORDER\n");
	}
#endif
	int dimx=hist->mat.dim[0].size;
	int dimy=hist->mat.dim[1].size;
#ifdef DEBUG
	printf("x=%d\ty=%d\n", dimx, dimy);
#endif
	double value;
	int count=0;
	float* bf = new float[dimx * dimy];
	for(int x=0; x<dimx; x++){
		for(int y=0; y<dimy; y++){
			value =  cvQueryHistValue_2D(
					hist,
					x,
					y
					);
			if(value>0){
				bf[count++]=value;
			}
		}
	}
#ifdef DEBUG
	printf("count:%d\n",count);
#endif

	int keep_count = (int)(count * scale);
	float threshold;
	if(order == ASCENDING_ORDER){
		qsort(bf, count, sizeof(float), compareAscendingF);
	}else{
		qsort(bf, count, sizeof(float), compareDescendingF);
	}
	threshold = bf[keep_count-1];
#ifdef DEBUG
	printf("keep_count:%d\tthreshold: %f\n",keep_count,threshold);
#endif

	float* needle;	
	count = 0;
	for(int x=0; x<dimx; x++){
		for(int y=0; y<dimy; y++){
			needle =  cvGetHistValue_2D(
					hist,
					x,
					y
					);
			if(order == ASCENDING_ORDER && *needle>=threshold){
				*needle = 0;
				count++;
			}else if(order == DESCENDING_ORDER && *needle!=0 && *needle<=threshold){
				*needle = 0;
				count++;
			}
		}
	}
#ifdef DEBUG
	printf("change %d nodes\n", count);
	printf("----------------------------end-----------------------------\n\n");
#endif
	delete bf;
	return 0;
}

/* *
 * will create CvMat header for mat_tmp in this function
 * hist should been normalized first!
 * */
int jkGetCvmatFromHist(const CvHistogram* hist, CvMat** mat){
	int dims = hist->mat.dims;
	if(dims != 2) return -1;

	int rows = hist->mat.dim[0].size;
	int cols = hist->mat.dim[1].size;
	*mat = cvCreateMat(rows, cols, CV_32FC1);
	cvSetZero(*mat);
	float bin_value;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			bin_value = cvQueryHistValue_2D(hist, i, j);
			*((float *)CV_MAT_ELEM_PTR(**mat, i, j)) = bin_value;
		}
	}

	return 0;
}

int jkSetCvMat2Hist(CvHistogram* hist, const CvMat* mat){
	int dims = hist->mat.dims;
	if(dims != 2) return -1;

	int rows = hist->mat.dim[0].size;
	int cols = hist->mat.dim[1].size;
	float bin_value;
	float* needle;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			bin_value = *((float *)CV_MAT_ELEM_PTR(*mat, i, j));
			needle = cvGetHistValue_2D(hist, i, j);
			*needle = bin_value;
		}
	}

	return 0;
}

void showConst(){
	printf("CV_8UC1\t%d\n", CV_8UC1);
	printf("CV_8UC2\t%d\n", CV_8UC2);
	printf("CV_8UC3\t%d\n", CV_8UC3);

	printf("CV_16SC1\t%d\n", CV_16SC1);
	printf("CV_16SC2\t%d\n", CV_16SC2);
	printf("CV_16SC3\t%d\n", CV_16SC3);
}

void checkdiff(IplImage* img){
	int width = img->width;
	int height = img->height;
	CvScalar cs;
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			cs = cvGet2D(img, i, j);
			printf("%f,%f,%f,%f\n", cs.val[0], cs.val[1], cs.val[2], cs.val[3]);
		}
	}
}

IplImage* showMat(const CvMat* mat){
	IplImage* img = cvCreateImage(cvGetSize(mat), IPL_DEPTH_8U, 1);
	cvConvertScaleAbs(mat, img, 1, 0);
	return img;
}

/* *
 * get one layer of img, from the contour_mask
 * note: 
 * 	contour_mask will change in this function
 * 	mat and contour_mask should has the same size
 * 	mat come from jkGetCvmatFromHist()
 * 	mat's type should be CV_32FC1
 * 	contour_mask' (depths,channels) should be (8,1) 
 * */
IplImage* getLayer(IplImage** img, CvHistogram* hist, CvMat* mat, IplImage* contour_mask){
	IplImage* layer = cvCreateImage(cvGetSize(img[0]), 8, 1);
	CvHistogram* hist_tmp = NULL;
	cvCopyHist(hist, &hist_tmp);
	CvMat* mat_tmp = cvCloneMat(mat);
	
	unsigned mask_v;
	uchar* ptr;
	for(int i=0; i < mat->height; i++){
		ptr = (uchar*)(
				contour_mask->imageData + i*contour_mask->widthStep
			      );
		for(int j=0; j<mat->width; j++){
			mask_v = *((unsigned*)(ptr+j));
			if(mask_v==0){
				cvmSet(mat_tmp, i, j, 0.0);
#ifdef DEBUG
				//printf("set mat_tmp (%d,%d) to 0\n", i, j);
#endif
			}else{
#ifdef DEBUG
				printf("keep mat_tmp (%d,%d)\n", i, j);
#endif
			}
		}
	}

	jkSetCvMat2Hist(hist_tmp, mat_tmp); 
	cvCalcBackProject(
			img,
			layer,
			hist_tmp
			);
	
	
	cvReleaseMat(&mat_tmp);
	cvReleaseHist(&hist_tmp);
	
	return layer;
}

int main( int argc, char** argv ){
#ifdef DEBUG
	showConst();
#endif
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	cvNamedWindow("Picture-in", 0);
	cvShowImage("Picture-in", src);

	//cvSmooth(src, src, CV_GAUSSIAN, 3, 3);
	//cvNamedWindow("gaussian smooth", 0);
	//cvShowImage("gaussian smooth", src);

	IplImage* hsv = cvCreateImage(cvGetSize(src), 8, 3);
	cvCvtColor(src, hsv, CV_BGR2HSV_FULL);
IplImage* h_plane = cvCreateImage(cvGetSize(src), 8, 1); IplImage* s_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* planes[] = {h_plane, v_plane};
	cvSplit(hsv, h_plane, s_plane, v_plane, 0);

	int h_bins = 180, v_bins = 100;
	CvHistogram* hist;
	
	{
		int hist_size[] = {h_bins, v_bins};
		float h_ranges[] = {0, 256};
		float v_ranges[] = {0, 256};
		float* ranges[] = {h_ranges, v_ranges};
		hist = cvCreateHist(
				2,
				hist_size,
				CV_HIST_ARRAY,
				ranges,
				1
				);
	}
	
	cvCalcHist(planes, hist, 0, 0);
	//cvThreshHist(hist, 20);
	//cvNormalizeHist(hist, 1.0);

	

	/******************** segmentation the picture from the histogram ***************************/
	CvMat* mat_tmp;
	jkGetCvmatFromHist(hist, &mat_tmp);// need no create CvMat header for mat_tmp
	cvNamedWindow("mt0", 0);
	cvShowImage("mt0", mat_tmp);
	//cvSmooth(mat_tmp, mat_tmp, CV_GAUSSIAN, 3, 3);//note: not all smooth method support in place(src=dst)
	
	CvMat* mat_tmp1 = cvCloneMat(mat_tmp);
	/*********************** stage 1 ***********************/
	// get the appropriate mat 
	cvThreshold(mat_tmp, mat_tmp, 20, 255, CV_THRESH_BINARY);
	cvNamedWindow("mt1", 0);
	cvShowImage("mt1", mat_tmp);

	cvDilate(mat_tmp, mat_tmp, NULL, 1);
	cvNamedWindow("mt2", 0);
	cvShowImage("mt2", mat_tmp);

	cvErode(mat_tmp, mat_tmp, NULL, 1);
	cvNamedWindow("mt3", 0);
	cvShowImage("mt3", mat_tmp);

	cvErode(mat_tmp, mat_tmp, NULL, 1);
	cvNamedWindow("mt4", 0);
	cvShowImage("mt4", mat_tmp);

	cvDilate(mat_tmp, mat_tmp, NULL, 1);
	cvNamedWindow("mt5", 0);
	cvShowImage("mt5", mat_tmp);
	


	/*********************** stage 2 ***********************/
	// find the huge pixel in histogram, add to mat_tmp
	cvThreshold(mat_tmp1, mat_tmp1, 500, 255, CV_THRESH_BINARY);
	cvDilate(mat_tmp1, mat_tmp1, NULL, 1);
	cvNamedWindow("mtb1", 0);
	cvShowImage("mtb1", mat_tmp1);

	cvOr(mat_tmp, mat_tmp1, mat_tmp, NULL);
	cvNamedWindow("mtb2", 0);
	cvShowImage("mtb2", mat_tmp);
	
	cvReleaseMat(&mat_tmp1);

	/*********************** stage 3 ***********************/
	// get the contours of the binary mat
	cvNamedWindow("contours", 0);
	IplImage* img_8uc1 = cvCreateImage(cvGetSize(mat_tmp), 8, 1);
	IplImage* contour_mask = cvCreateImage(cvGetSize(mat_tmp), 8, 1);
	cvConvertScaleAbs(mat_tmp, img_8uc1, 1, 0);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* first_contour = NULL;
	int Nc = cvFindContours(img_8uc1, storage, &first_contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	cvReleaseImage(&img_8uc1);
#ifdef DEBUG
	printf("total contours detected: %d\n", Nc);
#endif
	cvNamedWindow("layer", 0);
	for(CvSeq* c = first_contour; c!=NULL; c=c->h_next){
		cvSetZero(contour_mask);
		cvDrawContours(
				contour_mask,
				c,
				CVX_WHITE,
				CVX_WHITE,
				0,
				-1,
				8
			      );
		cvShowImage("contours", contour_mask);
		IplImage* layer = getLayer(planes, hist, mat_tmp, contour_mask);
		cvShowImage("layer", layer);
		
		cvWaitKey(0);
		cvDestroyWindow("layer");
	}
	cvReleaseImage(&contour_mask);
	

	//jkSetCvMat2Hist(hist, mat_tmp); 
	cvReleaseMat(&mat_tmp);

	//jkThreshHist2D(hist, 0.9, DESCENDING_ORDER);
	



	int scale = 4;
	IplImage* hist_img = cvCreateImage(
			cvSize(h_bins * scale, v_bins * scale),
			8,
			1
			);
	cvZero(hist_img);

	float max_value = 0;
	float min_value = 0;

	cvGetMinMaxHistValue(hist, &min_value, &max_value, 0, 0);
#ifdef DEBUG
	printf("min:%f\tmax:%f\n", min_value, max_value);
#endif

	for(int h=0; h<h_bins; h++){
		for(int v=0; v<v_bins; v++){
			float bin_value = cvQueryHistValue_2D(hist, h, v);
			
			/*
			if(bin_value>0){
				printf("h,v,bin_value = %d,%d,%f\n", h, v, bin_value);
			}
			*/
			
			int intensity = cvRound(bin_value * 255 / max_value);
			cvRectangle(
					hist_img,
					cvPoint(h*scale, v*scale),
					cvPoint((h+1)*scale-1, (v+1)*scale-1),
					CV_RGB(intensity, intensity, intensity),
					CV_FILLED
				   );
		}
	}


	cvNamedWindow("h-v histogram", 0);
	cvShowImage("h-v histogram", hist_img);

	


	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	//cvDestroyWindow("gaussian smooth");
	cvDestroyWindow("h-v histogram");
	cvDestroyWindow("mt1");
	cvDestroyWindow("mt2");
	cvDestroyWindow("mt3");
	cvDestroyWindow("contours");
	return 0;
}
