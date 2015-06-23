#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"

#define CVX_WHITE  CV_RGB(255, 255, 255)
#define CVX_RED  CV_RGB(255, 0, 0)
#define CVX_BLUE  CV_RGB(0, 0, 255)
#define CVX_GREEN  CV_RGB(0, 255, 0)
extern IplConvKernel* KERNEL_3R1C;
extern IplConvKernel* KERNEL_9R1C;
extern IplConvKernel* KERNEL_1R3C;

typedef struct JkImageList
{
	IplImage* img;
	struct JkImageList* next;
}
JkImageList;

typedef struct JkArrList
{
	CvArr* arr;
	struct JkArrList* next;
}
JkArrList;

typedef struct JkMatList
{
	CvMat* mat;
	bool has_char;
	struct JkMatList* next;
}
JkMatList;

typedef struct JkBoxList
{
	CvBox2D box;
	bool available;
	CvPoint2D32f boxPointsf[4];
	int* corners;//length should be 16
	int* pixels;//length should be 16
	float sinTheta;
	float cosTheta;
	struct JkBoxList* next;
}
JkBoxList;

typedef struct JkFeatureLayer{
	CvMat* mat;
	CvPoint2D32f* corners;
	int corner_count;
}
JkFeatureLayer;


JkFeatureLayer* jkFeaturePoint(IplImage* img);
JkMatList* jkGetColorLayer(IplImage* img);
JkBoxList* jkGetBox(CvMat* mask);
bool jkJudgeLayer(IplImage* img, CvMat* mask, JkBoxList* box_list, JkFeatureLayer* feature_layer);
int jkScalarBigger(const CvScalar a, const CvScalar b);
int jkScalarDiff(const CvScalar a, const CvScalar b, CvScalar* c);
CvMat* jkMinus1(CvMat* mat);
void jkSizeFilterBoxList(IplImage* img, JkBoxList* box_list);
int jkInsertPoint2BoxList(JkBoxList* box_list, CvPoint p);


void jkShowBoxMsg(CvBox2D box);
void jkShowBox(CvArr* img, CvBox2D box);
void jkShowBoxList(IplImage* img, JkBoxList* box_list);
void jkShowImageDetail(IplImage* img);
IplImage* jkMerge(IplImage* img, CvMat* mask_final);
void jkReleaseBoxList(JkBoxList** box_list);
void jkReleaseMatList(JkMatList** mat_list);
void jkReleaseFeatureLayer(JkFeatureLayer** feature_layer);
