#include<stdio.h>
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"

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

int main( int argc, char** argv ){
	IplImage* src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	cvNamedWindow("Picture-in", CV_WINDOW_AUTOSIZE);
	cvShowImage("Picture-in", src);

	//cvSmooth(src, src, CV_GAUSSIAN, 3, 3);
	cvNamedWindow("gaussian smooth", CV_WINDOW_AUTOSIZE);
	cvShowImage("gaussian smooth", src);

	IplImage* hsv = cvCreateImage(cvGetSize(src), 8, 3);
	cvCvtColor(src, hsv, CV_BGR2HSV);

	IplImage* h_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* s_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* planes[] = {h_plane, v_plane};
	cvSplit(hsv, h_plane, s_plane, v_plane, 0);

	int h_bins = 180, s_bins = 100;
	CvHistogram* hist;
	
	{
		int hist_size[] = {h_bins, s_bins};
		float h_ranges[] = {0, 180};
		float s_ranges[] = {0,255};
		float* ranges[] = {h_ranges, s_ranges};
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
	cvNormalizeHist(hist, 1.0);

	/*	
	CvMat* mat_tmp;
	jkGetCvmatFromHist(hist, &mat_tmp);// need no create CvMat header for mat_tmp
	cvSmooth(mat_tmp, mat_tmp, CV_GAUSSIAN, 3, 3);//note: not all smooth method support in place(src=dst)
	jkSetCvMat2Hist(hist, mat_tmp); 
	cvReleaseMat(&mat_tmp);

	jkThreshHist2D(hist, 0.9, DESCENDING_ORDER);
	*/
	

	int scale = 4;
	IplImage* hist_img = cvCreateImage(
			cvSize(h_bins * scale, s_bins * scale),
			8,
			3
			);
	cvZero(hist_img);

	float max_value = 0;
	float min_value = 0;
	cvGetMinMaxHistValue(hist, &min_value, &max_value, 0, 0);
#ifdef DEBUG
	printf("min:%f\tmax:%f\n", min_value, max_value);
#endif

	for(int h=0; h<h_bins; h++){
		for(int s=0; s<s_bins; s++){
			float bin_value = cvQueryHistValue_2D(hist, h, s);
			int intensity = cvRound(bin_value * 255 / max_value);
			cvRectangle(
					hist_img,
					cvPoint(h*scale, s*scale),
					cvPoint((h+1)*scale-1, (s+1)*scale-1),
					CV_RGB(intensity, intensity, intensity),
					CV_FILLED
				   );
		}
	}


	cvNamedWindow("h-s histogram", CV_WINDOW_AUTOSIZE);
	cvShowImage("h-s histogram", hist_img);

	cvWaitKey(0);
	cvDestroyWindow("Picture-in");
	cvDestroyWindow("gaussian smooth");
	cvDestroyWindow("h-s histogram");
	return 0;
}
