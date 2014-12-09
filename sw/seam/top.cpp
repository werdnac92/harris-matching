#include "hls_stream.h"
#include "hls_video.h"

#define MAX_WIDTH  300
#define MAX_HEIGHT 300

typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC3> COLOR;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_8UC1> GRAY;
typedef hls::Mat<MAX_HEIGHT,MAX_WIDTH,HLS_32FC1> FLOATIMG;
typedef hls::Scalar<1, unsigned char> PIXEL;

// *************** may have problems with bitwidths; might have to use 8 bit instead of 32 bit fifos


void hlscomputeGradient( GRAY& src, FLOATIMG& dst, int direction )
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


int hlsf2i(float x ){
    int total= 0;
    if (x<0) x=-x;
    while (x>0){
          x= x-1;
          total= total+1;
    }
    return total;
}

int hlscomputeHarris(GRAY& img1_gray, float harrisImage[MAX_HEIGHT*MAX_WIDTH], float orientationImage[MAX_HEIGHT*MAX_WIDTH], unsigned char threshedHarris[MAX_HEIGHT*MAX_WIDTH]){

  FLOATIMG grad_x(MAX_HEIGHT, MAX_WIDTH);
  FLOATIMG grad_y(MAX_HEIGHT, MAX_WIDTH);
  GRAY img1_gray1(MAX_HEIGHT, MAX_WIDTH);
  GRAY img1_gray2(MAX_HEIGHT, MAX_WIDTH);
  hls::Duplicate(img1_gray, img1_gray1, img1_gray2);
  
  hlscomputeGradient(img1_gray1, grad_x, 0);
  hlscomputeGradient(img1_gray2, grad_y, 1);

  int grad_x_vec[MAX_HEIGHT*MAX_WIDTH];
  int grad_y_vec[MAX_HEIGHT*MAX_WIDTH];
  for (int i = 0; i < MAX_HEIGHT*MAX_WIDTH; i++) {
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
  
  for (int y = 0; y < MAX_HEIGHT; y++) {
      for (int x = 0; x < MAX_WIDTH; x++) {
          float a = 0;
          float b = 0;
          float c = 0;
          
          int imgL= fmax(0, x-gaus_ks);
          int imgR= fmin(MAX_WIDTH-1, x+gaus_ks);
          int imgT= fmax(0, y-gaus_ks);
          int imgB= fmin(MAX_HEIGHT-1, y+gaus_ks);
          
          for (int i=imgT; i<=imgB; i++){
              for (int j=imgL; j<=imgR; j++){
                  a= a+grad_x_vec[i*MAX_WIDTH+j]*grad_x_vec[i*MAX_WIDTH+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
                  c= c+grad_y_vec[i*MAX_WIDTH+j]*grad_y_vec[i*MAX_WIDTH+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
                  b= b+grad_x_vec[i*MAX_WIDTH+j]*grad_y_vec[i*MAX_WIDTH+j]*gaussian5x5[(gaus_ks-(y-i))*5+gaus_ks-(x-j)];
              }
          }
           
           if ((a+c)==0) {
              harrisImage[y*MAX_HEIGHT+x]= 0;
           } else {
             harrisImage[y*MAX_HEIGHT+x]= (a * c - b * b)/(a + c);
           }
           
           double ang= atanf((c - a + sqrt((a-c)*(a-c) + 4 * b*b))/(2*b));
           double num= (c - a + sqrt((a-c)*(a-c) + 4 * b*b));
           double denom= 2*b;
           
           if (denom==0 && a==0){
              orientationImage[y*MAX_WIDTH+x]= 0;
           }else if(num>0 && denom>0){
              orientationImage[y*MAX_WIDTH+x]= ang*57.2957795;
           }else if(num<0 && denom>0){
                 orientationImage[y*MAX_WIDTH+x]= ang*57.2957795+360;
           }else if(num>0 && denom<0){
                 orientationImage[y*MAX_WIDTH+x]= ang*57.2957795+180;
           }else if(num<0 && denom<0){
                 orientationImage[y*MAX_WIDTH+x]= ang*57.2957795+180;
           }
          //printf("\n a=%f, b=%f, c=%f, num=%f, denom=%f, ang=%f \n", a, b, c, num, denom, ang);
          
          float val= orientationImage[y*MAX_WIDTH+x];
          
          if (val>0){
            //printf("(%i, %i), a=%f, b=%f, c=%f, num=%f, denom=%f, ang=%f, val=%f\n", x, y, a, b, c, num, denom, ang, val);
          }  
      }
  }
  
  int numFeatures= 0;
  for (int y = 0; y < MAX_HEIGHT; y++) {
    for (int x = 0; x < MAX_WIDTH; x++) {
          threshedHarris[y*MAX_WIDTH+x]= 0;
          if (harrisImage[y*MAX_WIDTH+x]>4000){
             threshedHarris[y*MAX_WIDTH+x]= 255;
             int x_start = fmax(0, x - 2);
             int y_start = fmax(0, y - 2);
             int x_stop = fmin(x + 2, MAX_WIDTH);
             int y_stop = fmin(y + 2, MAX_HEIGHT);
             
                             
             for (int k = y_start; k < y_stop; k++){
                 for (int l = x_start; l < x_stop; l++){
                     if (harrisImage[k*MAX_WIDTH+l] > harrisImage[y*MAX_WIDTH+x]){
                        threshedHarris[y*MAX_WIDTH+x]= 0; //not max
                     } 
                 }
              }
              if (threshedHarris[y*MAX_WIDTH+x]== 255){
                numFeatures++;
              }
          }
          
    }
  }
   
  return(numFeatures);
}

void read_stream(hls::stream<int>& in_stream, GRAY& out_mat) {
//void read_stream(hls::stream<float>& in_stream, GRAY& out_mat) {

  PIXEL pixel;

  for (int i=0; i<MAX_HEIGHT*MAX_WIDTH; i++) {
    pixel.val[0] = in_stream.read();
    out_mat << pixel;
  }

}

void write_stream(unsigned char threshed[], float orientation[], int numFeatures, hls::stream<int>& stream) {
//void write_stream(unsigned char threshed[], float orientation[], int numFeatures, hls::stream<float>& stream) {

  // Write the threshold harris image to stream
  for (int i=0; i<MAX_HEIGHT*MAX_WIDTH; i++) {
    stream << threshed[i];
  }

  // Write the angles of the image to stream
  for (int j=0; j<MAX_HEIGHT*MAX_WIDTH; j++) {
    stream << orientation[j];
  }

  stream << numFeatures;


}

void dut(hls::stream<int>& in_fifo, hls::stream<int>& out_fifo) {
//void dut(hls::stream<float>& in_fifo, hls::stream<float>& out_fifo) {

  #pragma HLS interface ap_ctrl_none port=return

  GRAY image1;

  read_stream(in_fifo, image1);

  float harrisImage1[MAX_HEIGHT*MAX_WIDTH];
  float orientationImage1[MAX_HEIGHT*MAX_WIDTH];
  unsigned char threshedHarris1[MAX_HEIGHT*MAX_WIDTH];
  int numFeatures1= 0;

  /*
  float harrisImage2[MAX_HEIGHT*MAX_WIDTH];
  float orientationImage2[MAX_HEIGHT*MAX_WIDTH];
  unsigned char threshedHarris2[MAX_HEIGHT*MAX_WIDTH];
  int numFeatures2= 0;
  */

  numFeatures1=hlscomputeHarris(image1, harrisImage1, orientationImage1, threshedHarris1);

  write_stream(threshedHarris1, orientationImage1, numFeatures1, out_fifo);

  //static int cnt;
  //in_fifo.read();
  //cnt++;
  //out_fifo.write(cnt);
}