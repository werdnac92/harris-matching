#ifndef _TOP_H_
#define _TOP_H_

#include "hls_video.h"

// maximum image size
#define MAX_WIDTH  500
#define MAX_HEIGHT 500

// I/O image settings
#define INPUT_IMAGE              "input.bmp"
#define OUTPUT_IMAGE             "output.bmp"
#define OUTPUT_IMAGE_GOLDEN      "output_golden.bmp"
#define OUTPUT_SEAM_IMAGE        "output_seam.bmp"
#define OUTPUT_SEAM_IMAGE_GOLDEN "output_seam_golden.bmp"

// typedef video library core structures
typedef hls::stream<ap_axiu<64,1,1,1> >            AXI_STREAM;
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_16UC1> GRAYSCALE_IMAGE;

void dut( hls::stream<int>& in_fifo, hls::stream<int>& out_fifo );

template<int ROWS, int COLS, int T>
void Stream2Mat( hls::stream<int>& strm, hls::Mat<ROWS, COLS, T>& img )
{
  int strm_pix;
  hls::Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;

  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;

  for( HLS_SIZE_T i = 0; i < rows; i++ ) {
#pragma HLS loop_tripcount max=ROWS
    for( HLS_SIZE_T j = 0; j < cols; j++ ) {
#pragma HLS loop_tripcount max=COLS
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1
      strm >> strm_pix;
      pix.val[0] = strm_pix;
      img << pix;
    }
  }
}

template<int ROWS, int COLS, int T>
void Mat2Stream( hls::Mat<ROWS, COLS, T>& img, hls::stream<int>& strm )
{
  hls::Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;

  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;

  for( HLS_SIZE_T i = 0; i < rows; i++ ) {
#pragma HLS loop_tripcount max=ROWS
    for( HLS_SIZE_T j = 0; j < cols; j++ ) {
#pragma HLS loop_tripcount max=COLS
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1
      img >> pix;
      strm << pix.val[0];
    }
  }
}

#endif
