//#include "top.h"
#include "opencv_top.h"
#include "/opt/xilinx/Vivado_HLS/2013.2/include/hls_opencv.h"
#include "/opt/xilinx/Vivado_HLS/2013.2/include/hls_video.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include <stdio.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "math.h"

#define MAX_WIDTH  500
#define MAX_HEIGHT 500

typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC3> COLOR;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC1> GRAY;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_32FC1> FLOATIMG;
typedef hls::Scalar<1, unsigned char> PIXEL;

//template<HLS_RGB2GRAY, 140, 214, HLS8UC3, HLS_8UC1>;
//hls::CvtColor<HLS_RGB2GRAY, 140, 214, HLS8UC3, HLS_8UC1>(hls::Mat<ROWS, COLS, SRC_T>& src, hls::Mat<ROWS, COLS, SRC_T>& dst);


void rotate_image(cv::Mat src, int centerX, int centerY, float rotationAngle, unsigned char feature[25]){
  //cv::Mat src;
  cv::Mat new_img_mat;

  cv::Mat rot_mat(2, 3, CV_32FC1);
  //Read in the input image
  //src = cv::imread("input.bmp", 1);

  //Set rotation angle, and calculate size of new image
  //double rotationAngle = 100.0;
  double scale = 1.0;

  double radians = rotationAngle * M_PI/180;
  double sinAngle = fabs(sinf(radians));
  double cosAngle = fabs(cosf(radians));
  int newWidth = (int) (((double)src.cols)*cosAngle + ((double)src.rows)*sinAngle);
  int newHeight = (int) (((double)src.cols)*sinAngle + ((double)src.rows)*cosAngle);
  //printf("old height %i, new height %i, old width %i, new width %i, sinangle %.5f, cosangle %.5f\n", src.rows, newHeight, src.cols, newWidth, sinAngle, cosAngle);

  //Rotate around the center of new image
  //cv::Size newSize = cv::Size(newWidth, newHeight);

  rot_mat = getRotationMatrix2D(cv::Point(centerX, centerY), rotationAngle, scale);

/*
  //Shift image to middle of destination
  double wdiff = (newWidth-src.cols)/2.0;
  double hdiff = (newHeight-src.rows)/2.0;
  rot_mat.at<double>(0,2) += wdiff;
  rot_mat.at<double>(1,2) += hdiff;*/

  //warpAffine(src, new_img_mat, rot_mat, newSize, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
  warpAffine(src, new_img_mat, rot_mat, src.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);

  if(centerX==111 && centerY==0){
    cv::imwrite("input_rotated2.bmp", new_img_mat(cv::Rect(centerX-2, centerY, 5,3)));
  }
  for(int y = -2; y <= 2; y++){
    for(int x = -2; x <= 2; x++){
      int imgX = centerX + x;
      int imgY = centerY + y;
      if(imgX < 0 || imgX > src.cols-1 || imgY < 0 || imgY > src.rows-1){
        feature[(y+2)*5 + (x+2)] = (unsigned char) 0;
      }else{
        //cv::mat.at(row, col)
        feature[(y+2)*5 + (x+2)] = new_img_mat.at<unsigned char>(imgY, imgX);
        //feature[(y+2)*src.cols + (x+2)] = src.at<unsigned char>(y,x);
      }
    }
  }
  //printf("Feature[0] = %i, Feature[4] = %i, Feature[8] = %i, Feature[24] = %i\n", feature[0], feature[4], feature[8], feature[24]);

  /*
  for(int i = 0; i < 25; i++){
    feature[i] = (unsigned char)8;
  }*/

}

void computeGradient( GRAY& src, FLOATIMG& dst, int direction )
{
  /*const float k_valx[3][3] = { {-0.125, 0, 0.125},
                             {-0.250, 0, 0.250},
                             {-0.125, 0, 0.125}, };
  
  const float k_valy[3][3] = { {0.125,0.250,0.125},
                             { 0, 0, 0},
                             {-0.125,-0.250,-0.125}, };*/
  
  const int k_valx[3][3] = { {-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}, };
  
  const int k_valy[3][3] = { {1,2,1},
                             { 0, 0, 0},
                             {-1,-2,-1}, };

                             
  hls::Window<3, 3, int> kernel;
  hls::Point_<int> anchor;

  if( direction == 0 ) {
    kernel.val[0][0] = k_valx[0][0];
    kernel.val[0][1] = k_valx[0][1];
    kernel.val[0][2] = k_valx[0][2];

    kernel.val[1][0] = k_valx[1][0];
    kernel.val[1][1] = k_valx[1][1];
    kernel.val[1][2] = k_valx[1][2];

    kernel.val[2][0] = k_valx[2][0];
    kernel.val[2][1] = k_valx[2][1];
    kernel.val[2][2] = k_valx[2][2];
  }else {
    kernel.val[0][0] = k_valy[0][0];
    kernel.val[0][1] = k_valy[0][1];
    kernel.val[0][2] = k_valy[0][2];

    kernel.val[1][0] = k_valy[1][0];
    kernel.val[1][1] = k_valy[1][1];
    kernel.val[1][2] = k_valy[1][2];

    kernel.val[2][0] = k_valy[2][0];
    kernel.val[2][1] = k_valy[2][1];
    kernel.val[2][2] = k_valy[2][2];
  }

  anchor.x = -1;
  anchor.y = -1;

  hls::Filter2D( src, dst, kernel, anchor );
  
}


int f2i(float x ){
    int total= 0;
    if (x<0) x=-x;
    while (x>0){
          x= x-1;
          total= total+1;
    }
    return total;
}

int computeHarris(GRAY& img1_gray, float harrisImage[MAX_HEIGHT*MAX_WIDTH], float orientationImage[MAX_HEIGHT*MAX_WIDTH], unsigned char threshedHarris[MAX_HEIGHT*MAX_WIDTH]){
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;
  FLOATIMG grad_x(rows, cols);
  FLOATIMG grad_y(rows, cols);
  GRAY img1_gray1(rows, cols);
  GRAY img1_gray2(rows, cols);
  hls::Duplicate(img1_gray, img1_gray1, img1_gray2);
  
  computeGradient(img1_gray1, grad_x, 0);
  computeGradient(img1_gray2, grad_y, 1);

  int grad_x_vec[rows*cols];
  int grad_y_vec[rows*cols];
  for (int i = 0; i < rows*cols; i++) {
      grad_x_vec[i]= grad_x.read().val[0];
      grad_y_vec[i]= grad_y.read().val[0];
  }
  
  int gaus_ks = 1;
  const double gaussian5x5[25] =
  {
  0.003663, 0.014652,  0.025641,  0.014652,  0.003663, 
  0.014652, 0.0586081, 0.0952381, 0.0586081, 0.014652, 
  0.025641, 0.0952381, 0.150183,  0.0952381, 0.025641, 
  0.014652, 0.0586081, 0.0952381, 0.0586081, 0.014652, 
  0.003663, 0.014652,  0.025641,  0.014652,  0.003663
  };
  
  for (int y = 0; y < rows; y++) {
      for (int x = 0; x < cols; x++) {
          float a = 0;
          float b = 0;
          float c = 0;
          
          int imgL= fmax(0, x-gaus_ks);
          int imgR= fmin(cols-1, x+gaus_ks);
          int imgT= fmax(0, y-gaus_ks);
          int imgB= fmin(rows-1, y+gaus_ks);
          
          for (int i=imgT; i<=imgB; i++){
              for (int j=imgL; j<=imgR; j++){
                  a= a+grad_x_vec[i*cols+j]*grad_x_vec[i*cols+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
                  c= c+grad_y_vec[i*cols+j]*grad_y_vec[i*cols+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
                  b= b+grad_x_vec[i*cols+j]*grad_y_vec[i*cols+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
              }
          }
           
           if ((a+c)==0) {
              harrisImage[y*rows+x]= 0;
           } else {
             harrisImage[y*rows+x]= (a * c - b * b)/(a + c);
           }
           
           double ang= atanf((c - a + sqrt((a-c)*(a-c) + 4 * b*b))/(2*b));
           double num= (c - a + sqrt((a-c)*(a-c) + 4 * b*b));
           double denom= 2*b;
           
           if (denom==0 && a==0){
              orientationImage[y*cols+x]= 0;
           }else if(num>0 && denom>0){
              orientationImage[y*cols+x]= ang*57.2957795;
           }else if(num<0 && denom>0){
                 orientationImage[y*cols+x]= ang*57.2957795+360;
           }else if(num>0 && denom<0){
                 orientationImage[y*cols+x]= ang*57.2957795+180;
           }else if(num<0 && denom<0){
                 orientationImage[y*cols+x]= ang*57.2957795+180;
           }
          //printf("\n a=%f, b=%f, c=%f, num=%f, denom=%f, ang=%f \n", a, b, c, num, denom, ang);
          
          float val= orientationImage[y*cols+x];
          
          if (val>180){
          //printf("(%i, %i), a=%f, b=%f, c=%f, num=%f, denom=%f, ang=%f\n", x, y, a, b, c, num, denom, ang);
          }  
      }
  }
  
  int numFeatures= 0;
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
          threshedHarris[y*cols+x]= 0;
          if (harrisImage[y*cols+x]>4000){
             threshedHarris[y*cols+x]= 255;
             int x_start = fmax(0, x - 2);
             int y_start = fmax(0, y - 2);
             int x_stop = fmin(x + 2, cols);
             int y_stop = fmin(y + 2, rows);
             
                             
             for (int k = y_start; k < y_stop; k++){
                 for (int l = x_start; l < x_stop; l++){
                     if (harrisImage[k*cols+l] > harrisImage[y*cols+x]){
                        threshedHarris[y*cols+x]= 0; //not max
										 } 
                 }
              }
							if (threshedHarris[y*cols+x]== 255){
								numFeatures++;
							}
          }
          
    }
  }
	 
	return(numFeatures);
}



/*void extractFeatures(cv::Mat src, unsigned char threshedHarris[], float orientationImage[], unsigned char featureList[*][25]){
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;
  int count = 0;
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
      if(threshedHarris[y*cols+x] == 255){
        unsigned char feature [25];
        rotate_image(src, x, y, orientationImage[y*cols+x], feature);
        for(int i = 0; i < 25; i++){
          featureList[count][i] = feature[i];
        }
        count++;
      }
    }
  }
}*/


void import() {
  cv::Mat img1_cvmat = cv::imread( "input1.bmp", 1 );
  cv::Mat img2_cvmat = cv::imread( "input4.bmp", 1 );
  //IplImage* image_ipl = cvLoadImage( "square.bmp", CV_LOAD_IMAGE_COLOR );
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;
  
  COLOR img1(rows, cols);
  COLOR img2(rows, cols);
  cvMat2hlsMat(img1_cvmat, img1);
  cvMat2hlsMat(img2_cvmat, img2);
  
  GRAY img1_gray(rows, cols);
  GRAY img2_gray(rows, cols);
  FLOATIMG img1_harris(rows, cols);
  FLOATIMG img2_harris(rows, cols);
  
  hls::CvtColor<HLS_RGB2GRAY>(img1, img1_gray);
  hls::CvtColor<HLS_RGB2GRAY>(img2, img2_gray);
	
   
  GRAY img1_gray1(rows, cols);
  GRAY img1_gray2(rows, cols);
  GRAY img2_gray1(rows, cols);
  GRAY img2_gray2(rows, cols);
  hls::Duplicate(img1_gray, img1_gray1, img1_gray2);
  hls::Duplicate(img2_gray, img2_gray1, img2_gray2);
  
	unsigned char img1_gray_vec[rows*cols];
  unsigned char img2_gray_vec[rows*cols];
  for (int i = 0; i < rows*cols; i++) {
      img1_gray_vec[i]= img1_gray2.read().val[0];
      img2_gray_vec[i]= img2_gray2.read().val[0];
  }
	
  float harrisImage1[rows*cols];
  float orientationImage1[rows*cols];
  unsigned char threshedHarris1[rows*cols];
  int numFeatures1= 0;
  float harrisImage2[rows*cols];
  float orientationImage2[rows*cols];
  unsigned char threshedHarris2[rows*cols];
  int numFeatures2= 0;
  
  numFeatures1=computeHarris(img1_gray1, harrisImage1, orientationImage1, threshedHarris1);
  numFeatures2=computeHarris(img2_gray1, harrisImage2, orientationImage2, threshedHarris2);
	printf("numFeatures1=%i, numFeatures2=%i\n", numFeatures1, numFeatures2);
  
  unsigned char featureList1[numFeatures1][25];
  unsigned char featureList2[numFeatures2][25];
  
  //extractFeatures(img1_cvmat, threshedHarris1, orientationImage1, featureList1);

	
	cv::Mat out1 = cv::Mat::zeros(cols, rows,CV_8U);
	cv::Mat out2 = cv::Mat::zeros(cols, rows,CV_8U);
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
			out1.at<unsigned char>(y,x) = threshedHarris1[y*cols+x];
      out2.at<unsigned char>(y,x) = threshedHarris2[y*cols+x];
    }
  }
  printf("----------hello4--------\n");
  cv::imwrite("out1.bmp", out1);
	cv::imwrite("out2.bmp", out2);
  printf("----------hello5--------\n");
	


  cv::Mat img1_cvmat_gray = cv::imread("input1.bmp", 0);
  cv::Mat img2_cvmat_gray = cv::imread("input4.bmp", 0);
	
	int count = 0;
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
      if(threshedHarris1[y*cols+x] >0){
				printf("count=%i\n", count);
        unsigned char feature[25];
        rotate_image(img1_cvmat_gray, x, y, orientationImage1[y*cols+x], feature);
        printf("Processing feature %i. Feature[0] = %i, Feature[24] = %i\n", count, feature[0], feature[24]);
        if(count == 6){
          printf("Feature 6 found! x=%i, y=%i, angle=%.7f\n", x, y, orientationImage1[y*cols+x]);
        }
        for(int i = 0; i < 25; i++){
          featureList1[count][i] = feature[i];
          if(i==24){
            printf("Got here 0\n");
          }
        }
        count++;
      }
    }
  }
  printf("----------between--------\n");

  count = 0;
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
      if(threshedHarris2[y*cols+x] == 255){
        unsigned char feature [25];
        rotate_image(img2_cvmat_gray, x, y, orientationImage2[y*cols+x], feature);
        printf("Processing feature %i. Feature[0] = %i, Feature[24] = %i\n", count, feature[0], feature[24]);
        if(count == 6){
          printf("Feature 6 found! x=%i, y=%i, angle=%.7f\n", x, y, orientationImage2[y*cols+x]);
        }
        for(int i = 0; i < 25; i++){
          featureList2[count][i] = feature[i];
          if(i==24){
            printf("Got here 0\n");
          }
        }
        count++;
      }
    }
  }
  
  double matches_dist[numFeatures1];
  int matches_id1[numFeatures1];
  int matches_id2[numFeatures1];
  
  
  double d;
  double dBest;
	double dSecondBest;
  int idBest;
  
  
  for (int i=0; i<numFeatures1; i++) {
       dBest       = 100000000;
       dSecondBest = 100000000;
       idBest = 0;
                
       for (int j=0; j<numFeatures2; j++) {
					//-----------
					// compute the SSD distance between 2 features
					double dist = 0;
					for (int k=0; k<25; k++) {
							dist += (featureList1[i][k]-featureList2[j][k])*(featureList1[i][k]-featureList2[j][k]);
					}
					double d = sqrt(dist);
					//printf("i=%i, j=%i, d=%f\n", i,j, d);
					//---------------
					
					
					
					if (d < dBest) {
					dSecondBest = dBest;
					dBest = d;
					idBest = j;
					}
       }
       
       matches_id1[i] = i;
       matches_id2[i] = idBest;
       matches_dist[i]= dBest/dSecondBest;
  }
  
  
  
  for(int i = 0; i < numFeatures1; i++){
         printf("id1=%i, id2=%i, dist=%f. \n", matches_id1[i], matches_id2[i], matches_dist[i]);
  }
  
  
  
  
  cv::Mat out_img_mat = cv::Mat::zeros(5,5,CV_8U);
	cv::Mat out_img_mat2 = cv::Mat::zeros(5,5,CV_8U);
  for(int y = 0; y < 5; y++){
    for(int x = 0; x < 5; x++){
      out_img_mat.at<unsigned char>(y,x) = featureList1[0][y*5+x];
			out_img_mat2.at<unsigned char>(y,x) = featureList2[0][y*5+x];
    }
  }
  printf("----------hello4--------\n");
  cv::imwrite("rotatedfeature3.bmp", out_img_mat);
	cv::imwrite("rotatedfeature4.bmp", out_img_mat2);
  printf("----------hello5--------\n");

  

}



int main( int argc, char** argv )
{
  import();

  return 0;
}
