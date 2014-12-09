#include "opencv_top.h"
#include "top.h"

void opencv_image_filter( IplImage* src, CvMat** dst )
{
  IplImage* image_gray = cvCreateImage( cvGetSize( src ), IPL_DEPTH_16U, 1 );
  cvCvtColor( src, image_gray, CV_RGB2GRAY );

  CvMat* xGrad = cvCreateMat( src->height, src->width, CV_16UC1 );
  CvMat* yGrad = cvCloneMat( xGrad );

  cvSobel( image_gray, xGrad, 1, 0, 3 );
  cvSobel( image_gray, yGrad, 0, 1, 3 );

  cvAbs( xGrad, xGrad );
  cvAbs( yGrad, yGrad );
  cvAdd( xGrad, yGrad, xGrad );

  *dst = cvCloneMat( xGrad );

  cvReleaseMat( &xGrad );
  cvReleaseMat( &yGrad );
}
