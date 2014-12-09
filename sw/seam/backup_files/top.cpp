#include "top.h"

void computeGradient( GRAYSCALE_IMAGE& src, GRAYSCALE_IMAGE& dst, int direction )
{
  const int k_valx[3][3] = { {-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}, };
  
  const int k_valy[3][3] = { {-1,-2,-1},
                             { 0, 0, 0},
                             { 1, 2, 1}, };

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
  }
  else {
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

void process_input( hls::stream<int>& input, int& rows, int& cols, hls::stream<int>& image )
{
  input >> rows;
  input >> cols;

  for( int i = 0; i < rows*cols; i++ ) {
    image << input.read();
  }
}

void dut( hls::stream<int>& in_fifo, hls::stream<int>& out_fifo )
{
//#ifdef GEN_BITSTREAM
  #pragma HLS interface ap_ctrl_none port=return
//#endif

  int rows, cols;
  hls::stream<int> image;
  #pragma HLS stream variable=image depth=65536
  process_input( in_fifo, rows, cols, image );

  GRAYSCALE_IMAGE src( rows, cols );
  GRAYSCALE_IMAGE src1( rows, cols );
  GRAYSCALE_IMAGE src2( rows, cols );
  GRAYSCALE_IMAGE dst( rows, cols );

  GRAYSCALE_IMAGE x_grad( rows, cols );
  GRAYSCALE_IMAGE x_grad1( rows, cols );

  GRAYSCALE_IMAGE y_grad( rows, cols );
  GRAYSCALE_IMAGE y_grad1( rows, cols );

  GRAYSCALE_IMAGE zero( rows, cols );
  GRAYSCALE_IMAGE zero1( rows, cols );
  GRAYSCALE_IMAGE zero2( rows, cols );

#pragma HLS dataflow
  Stream2Mat( image, src );
  hls::Duplicate( src, src1, src2 );

  computeGradient( src1, x_grad, 0 );
  computeGradient( src2, y_grad, 1 );

  hls::Zero( zero );
  hls::Duplicate( zero, zero1, zero2 );

  hls::AbsDiff( x_grad, zero1, x_grad1 );
  hls::AbsDiff( y_grad, zero2, y_grad1 );

  hls::AddWeighted( x_grad1, 1, y_grad1, 1, 0, dst );

  Mat2Stream( dst, out_fifo );
}
