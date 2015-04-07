#include "opencv/cv.h"
#include "opencv/highgui.h"


int main(int argc, char** argv){
	IplImage* img = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	cvNamedWindow("Picture-in", CV_WINDOW_AUTOSIZE);
	cvShowImage("Picture-in", img);

	
	cvWaitKey(0);
	cvDestroyWindow("Picture-in");

	
	return 0;
}
