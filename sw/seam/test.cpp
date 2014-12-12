#include "top.h"
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

IplImage* test_img;

void rotate_image(cv::Mat src, int centerX, int centerY, float rotationAngle, unsigned char feature[25]){
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;
  
  cv::Mat new_img_mat;
  cv::Mat rot_mat(2, 3, CV_32FC1);

  double scale = 1.0;

  rot_mat = getRotationMatrix2D(cv::Point(centerX, centerY), rotationAngle, scale);

  //warpAffine(src, new_img_mat, rot_mat, newSize, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
  warpAffine(src, new_img_mat, rot_mat, src.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	
  for(int y = -3; y <= 3; y++){
    for(int x = -3; x <= 3; x++){
      int imgX = centerX + x;
      int imgY = centerY + y;

      if(imgX < 0 || imgX > cols || imgY < 0 || imgY > rows){
        feature[(y+3)*7 + (x+3)] = (unsigned char) 0;
      }else{
        //cv::mat.at(row, col)
        //feature[(y+2)*5 + (x+2)] = src.at<unsigned char>(y, x);
        feature[(y+3)*7 + (x+3)] = new_img_mat.at<unsigned char>(imgY, imgX);
        //printf("andrew: %u\n", new_img_mat.at<unsigned char>(imgY, imgX));
      }
    }
  }
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


int computeHarris(cv::Mat& fpga_hls_gray, float FPGA_angles[MAX_HEIGHT*MAX_WIDTH], unsigned char FPGA_threshed1[MAX_HEIGHT*MAX_WIDTH]){

  int           FPGA_numFeatures;
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;

  hls::stream<int> to_fpga;
  hls::stream<int> from_fpga;

  for (int r=0; r<rows; r++) {
    for (int c=0; c<cols; c++) {
      to_fpga << fpga_hls_gray.at<unsigned char>(r,c);
    }
  }

  dut(to_fpga, from_fpga);

  for (int j=0; j<rows*cols; j++){
    FPGA_threshed1[j]=from_fpga.read();
  }

  for (int k=0; k<rows*cols; k++){
    FPGA_angles[k]=from_fpga.read();
  }

  FPGA_numFeatures = from_fpga.read();

  printf("FPGA says # of features = %i\n", FPGA_numFeatures);

  return FPGA_numFeatures;
}


void import() {
  int rows= MAX_HEIGHT;
  int cols= MAX_WIDTH;

  cv::Mat gray1(cols, rows, CV_8U);
  cv::Mat gray2(cols, rows, CV_8U);

  gray1 = cv::imread("input1.bmp",0);
  gray2 = cv::imread("input4.bmp",0);


	int gray1_vec[rows*cols];
  int gray2_vec[rows*cols];

  for (int r=0; r<rows; r++) {
    for (int c=0; c<cols; c++) {
      gray1_vec[r*cols+c]= gray1.at<unsigned char>(r,c);
      gray2_vec[r*cols+c]= gray2.at<unsigned char>(r,c);
    }
  }

  float orientationImage1[rows*cols], orientationImage2[rows*cols];
  unsigned char threshedHarris1[rows*cols], threshedHarris2[rows*cols];

  int numFeatures1 = computeHarris(gray1, orientationImage1, threshedHarris1);
  int numFeatures2 = computeHarris(gray2, orientationImage2, threshedHarris2);

  unsigned char featureList1[numFeatures1][49];
  unsigned char featureList2[numFeatures2][49];

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
	


  int matches1_row[numFeatures1];
	int matches1_col[numFeatures1];
  int matches2_row[numFeatures2];
	int matches2_col[numFeatures2];
	
	int count = 0;
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
      if(threshedHarris1[y*cols+x] >0){
				//printf("count=%i\n", count);
        unsigned char feature[49];
        rotate_image(gray1, x, y, orientationImage1[y*cols+x], feature);
        //printf("Processing feature %i. x=%i, y=%i, angle=%.7f\n", count, x,y,  orientationImage2[y*cols+x]);
        /*if(count == 6){
          printf("Feature 6 found! x=%i, y=%i, angle=%.7f\n", x, y, orientationImage1[y*cols+x]);
        }*/
        for(int i = 0; i < 49; i++){
          featureList1[count][i] = feature[i];
          if(i==24){
            printf("Got here 0");
          }
        }
				
				matches1_row[count]= y; // row
				matches1_col[count]= x; // col
        count++;
      }
    }
  }
  printf("----------between--------\n");

  count = 0;
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
      if(threshedHarris2[y*cols+x] == 255){
        unsigned char feature [49];
        rotate_image(gray2, x, y, orientationImage2[y*cols+x], feature);
        //printf("Processing feature %i. x=%i, y=%i, angle=%.7f\n", count, x,y,  orientationImage2[y*cols+x]);
        /*if(count == 6){
          printf("Feature 6 found! x=%i, y=%i, angle=%.7f\n", x, y, orientationImage2[y*cols+x]);
        }*/
        for(int i = 0; i < 49; i++){
          featureList2[count][i] = feature[i];
          if(i==24){
            printf("Got here 0");
          }
        }
				
				matches2_row[count]= y; // row
				matches2_col[count]= x; // col
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
         printf("id1=%i (%i,%i), id2=%i (%i,%i), dist=%f. \n", matches_id1[i], matches1_row[i],  matches1_col[i], matches_id2[i],  matches2_row[i],  matches2_col[i],matches_dist[i]);
  }
  
	int quadrant0[4]={0,0,0,0};
	int quadrant1[4]={0,0,0,0};
	int quadrant2[4]={0,0,0,0};
	int quadrant3[4]={0,0,0,0};
	
	for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
			int x_start = fmax(0, x - 3);
			int y_start = fmax(0, y - 3);
			int x_stop = fmin(x + 3, cols);
			int y_stop = fmin(y + 3, rows);
			
			for (int i=0; i<numFeatures1; i++){
				if (y==matches1_row[i] && x==matches1_col[i] && matches_dist[i]<0.7){
					if (y<rows/2 && x<cols/2){ //quadrant top left 0
						if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant top left 0
							quadrant0[0]++;
						} else if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]>cols/2){ //quadrant top right 1 
							quadrant0[1]++;
						} else if (matches2_row[matches_id2[i]]>rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant bottom left 2
							quadrant0[2]++;
						} else{ //quadrant top right 3
							quadrant0[3]++;
						}
					} else if (y<rows/2 && x>cols/2){ //quadrant top right 1 
						if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant top left 0
							quadrant1[0]++;
						} else if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]>cols/2){ //quadrant top right 1 
							quadrant1[1]++;
						} else if (matches2_row[matches_id2[i]]>rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant bottom left 2
							quadrant1[2]++;
						} else{ //quadrant top right 3
							quadrant1[3]++;
						}
					} else if (y>rows/2 && x<cols/2){ //quadrant bottom left 2
						if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant top left 0
							quadrant2[0]++;
						} else if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]>cols/2){ //quadrant top right 1 
							quadrant2[1]++;
						} else if (matches2_row[matches_id2[i]]>rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant bottom left 2
							quadrant2[2]++;
						} else{ //quadrant top right 3
							quadrant2[3]++;
						}
					} else { //quadrant top right 3
						if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant top left 0
							quadrant3[0]++;
						} else if (matches2_row[matches_id2[i]]<rows/2 && matches2_col[matches_id2[i]]>cols/2){ //quadrant top right 1 
							quadrant3[1]++;
						} else if (matches2_row[matches_id2[i]]>rows/2 && matches2_col[matches_id2[i]]<cols/2){ //quadrant bottom left 2
							quadrant3[2]++;
						} else{ //quadrant top right 3
							quadrant3[3]++;
						}
					}
					for (int k = y_start; k <= y_stop; k++){
							for (int l = x_start; l <= x_stop; l++){
									 if (k==y_start || l==x_start || k==y_stop || l==x_stop){
											gray1_vec[k*cols+l]= 100;
									 } 
							}
					}
				}
				
				if (y==matches2_row[matches_id2[i]] && x==matches2_col[matches_id2[i]] && matches_dist[i]<0.7){
					for (int k = y_start; k <= y_stop; k++){
							for (int l = x_start; l <= x_stop; l++){
									 if (k==y_start || l==x_start || k==y_stop || l==x_stop){
											gray2_vec[k*cols+l]= 100;
									 } 
							}
					}
				}
			}
			
			
			
    }
  }
	
	printf("quadrant 0 :\n");
	int best_quadrants[4];
	int best_val= -1;
	int best_quad_in_img2= 0;
	for (int i=0; i<4; i++){
		int tmp= quadrant0[i];
		printf("%i has %i matches\n", i, tmp);
		if (tmp> best_val){
			best_val= tmp;
			best_quad_in_img2= i;
		}
	}
	best_quadrants[0]= best_quad_in_img2;
	
	printf("quadrant 1 :\n");
	best_val= -1;
	best_quad_in_img2= 0;
	for (int i=0; i<4; i++){
		int tmp= quadrant1[i];
		printf("%i has %i matches\n", i, tmp);
		if (tmp> best_val){
			best_val= tmp;
			best_quad_in_img2= i;
		}
	}
	best_quadrants[1]= best_quad_in_img2;
	
	printf("quadrant 2 :\n");
	best_val= -1;
	best_quad_in_img2= 0;
	for (int i=0; i<4; i++){
		int tmp= quadrant2[i];
		printf("%i has %i matches\n", i, tmp);
		if (tmp> best_val){
			best_val= tmp;
			best_quad_in_img2= i;
		}
	}
	best_quadrants[2]= best_quad_in_img2;
	
	printf("quadrant 3 :\n");
	best_val= -1;
	best_quad_in_img2= 0;
	for (int i=0; i<4; i++){
		int tmp= quadrant3[i];
		printf("%i has %i matches\n", i, tmp);
		if (tmp> best_val){
			best_val= tmp;
			best_quad_in_img2= i;
		}
	}
	best_quadrants[3]= best_quad_in_img2;
	
	printf("\nQ0 is top-left, Q1 is top-right, Q2 is bottom-left, Q3 is bottom-right.\n");
	printf("Q0 in img1->Q%i in img2\n", best_quadrants[0]);
	printf("Q1 in img1->Q%i in img2\n", best_quadrants[1]);
	printf("Q2 in img1->Q%i in img2\n", best_quadrants[2]);
	printf("Q3 in img1->Q%i in img2\n", best_quadrants[3]);
	
	unsigned char final_vec[rows*cols*2];
	
	for(int y = 0; y < rows; y++){
    for(int x = 0; x < cols; x++){
			final_vec[y*2*cols+x] = gray1_vec[y*cols+x];
			final_vec[y*2*cols+x+cols] = gray2_vec[y*cols+x];
    }
  }
	
	cv::Mat final = cv::Mat::zeros(rows, cols*2,CV_8U);
  for(int y = 0; y < rows; y++){
    for(int x = 0; x < 2*cols; x++){
			final.at<unsigned char>(y,x) = final_vec[y*2*cols+x];
    }
  }

  cv::imwrite("final.bmp", final);
  

}



int main( int argc, char** argv )
{
  import();

  return 0;
}
