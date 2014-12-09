/***************************************************************************

*   © Copyright 2013 Xilinx, Inc. All rights reserved. 

*   This file contains confidential and proprietary information of Xilinx,
*   Inc. and is protected under U.S. and international copyright and other
*   intellectual property laws. 

*   DISCLAIMER
*   This disclaimer is not a license and does not grant any rights to the
*   materials distributed herewith. Except as otherwise provided in a valid
*   license issued to you by Xilinx, and to the maximum extent permitted by
*   applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH
*   ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, 
*   EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES
*   OF MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR 
*   PURPOSE; and (2) Xilinx shall not be liable (whether in contract or 
*   tort, including negligence, or under any other theory of liability)
*   for any loss or damage of any kind or nature related to, arising under
*   or in connection with these materials, including for any direct, or any
*   indirect, special, incidental, or consequential loss or damage (including
*   loss of data, profits, goodwill, or any type of loss or damage suffered 
*   as a result of any action brought by a third party) even if such damage
*   or loss was reasonably foreseeable or Xilinx had been advised of the 
*   possibility of the same. 

*   CRITICAL APPLICATIONS 
*   Xilinx products are not designed or intended to be fail-safe, or for use
*   in any application requiring fail-safe performance, such as life-support
*   or safety devices or systems, Class III medical devices, nuclear facilities,
*   applications related to the deployment of airbags, or any other applications
*   that could lead to death, personal injury, or severe property or environmental
*   damage (individually and collectively, "Critical Applications"). Customer
*   assumes the sole risk and liability of any use of Xilinx products in Critical
*   Applications, subject only to applicable laws and regulations governing 
*   limitations on product liability. 

*   THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT 
*   ALL TIMES.

***************************************************************************/
#ifndef _HLS_VIDEO_FAST_H_
#define _HLS_VIDEO_FAST_H_
#include "hls_video.h"
#include "ap_fixed.h"
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))


template<int PSize,int KERNEL_SIZE,int NUM,typename SRC_T>
int fast_pixel_diff(
        hls::Window<KERNEL_SIZE,KERNEL_SIZE,SRC_T>  & win,
        int  _threshold,
        int (&flag_val)[NUM],
        int (&flag_d)[NUM],
        int (&flag)[PSize][2])
{
#pragma HLS INLINE
     int vt1,vt2,x1,x2,y1,y2;
     int result=3;
     int K=PSize/2;    
     for(int index=0;index<K;index++)
     {
         int kernel_half=KERNEL_SIZE/2;
         int K=(int)(PSize/2);
         x1=flag[index][0];
         y1=flag[index][1];
         x2=flag[index+K][0];
         y2=flag[index+K][1];
         vt1=win.val[kernel_half][kernel_half]-win.val[ y1 ][ x1 ];    
         vt2=win.val[kernel_half][kernel_half]-win.val[ y2 ][ x2 ];   
         flag_d[index]=vt1;
         flag_d[index+PSize/2]=vt2;
         flag_d[index+PSize]=vt1;
         if (vt1>_threshold)
             vt1=1;
         else if(vt1<-_threshold)
             vt1=2;
         else
             vt1=0;
         if (vt2>_threshold)
             vt2=1;
         else if(vt2<-_threshold)
             vt2=2;
         else
             vt2=0;
         result&= vt1|vt2;
         flag_val[index]=vt1;
         flag_val[index+K]=vt2;
         flag_val[index+PSize]=vt1;
     }
     flag_val[PSize/2+PSize]=flag_val[PSize/2];
     flag_d[PSize/2+PSize]=flag_d[PSize/2];
     return   result;
}  
template<int PSize,int NUM>
int coreScore(int (&flag_d)[NUM],int _threshold)
{
#pragma HLS INLINE

    short flag_d_min2[NUM-1];
    short flag_d_max2[NUM-1];
    short flag_d_min4[NUM-3];
    short flag_d_max4[NUM-3];
    short flag_d_min8[NUM-7];
    short flag_d_max8[NUM-7];
    if(PSize == 16) {
    for(int i=0;i<NUM-1;i++)
    {
        flag_d_min2[i] = MIN(flag_d[i],flag_d[i+1]);
        flag_d_max2[i] = MAX(flag_d[i],flag_d[i+1]);
    }

    for(int i=0;i<NUM-3;i++)
    {
        flag_d_min4[i] = MIN(flag_d_min2[i],flag_d_min2[i+2]);
        flag_d_max4[i] = MAX(flag_d_max2[i],flag_d_max2[i+2]);
    }

    for(int i=0;i<NUM-7;i++)
    {
        flag_d_min8[i] = MIN(flag_d_min4[i],flag_d_min4[i+4]);
        flag_d_max8[i] = MAX(flag_d_max4[i],flag_d_max4[i+4]);
    }
    } else {
        assert(false);
    }

    int a0=_threshold;
    for(int i=0;i<PSize;i+=2)
    {
        int a=255;
        if(PSize == 16) {
            a = flag_d_min8[i+1];
        } else {
            for(int j=1;j<PSize/2+1;j++)
                {
                    a=MIN(a,flag_d[i+j]);
                }
        }
        a0=MAX(a0,MIN(a,flag_d[i]));
        a0=MAX(a0,MIN(a,flag_d[i+PSize/2+1]));
    }
    int b0=-_threshold;
    for(int i=0;i<PSize;i+=2)
    {
        int b=-255;
        if(PSize == 16) {
            b = flag_d_max8[i+1];
        } else {
            for(int j=1;j<PSize/2+1;j++)
                {
                    b=MAX(b,flag_d[i+j]);
                }
        }
        b0=MIN(b0,MAX(b,flag_d[i]));
        b0=MIN(b0,MAX(b,flag_d[i+PSize/2+1]));
    }
    return MAX(a0,-b0)-1;
}

template<int PSize,int KERNEL_SIZE,int NUM,typename SRC_T>
bool fast_judge(
        hls::Window<KERNEL_SIZE,KERNEL_SIZE,SRC_T>  & win,
        int  _threshold,
        int (&flag_val)[NUM],
        int (&flag_d)[NUM],
        int (&flag)[PSize][2],
        int &core,
        bool  _nonmax_supression
        )
{
#pragma HLS INLINE
    bool iscorner=false;
    int kernel_half=KERNEL_SIZE/2;
    int result=fast_pixel_diff<PSize>(win,_threshold,flag_val,flag_d,flag);
    int count=1;
//     if(result==1||result==2)
//     {
//        bool is_onece=true;
        
        for(int c=1;c<PSize+PSize/2+1;c++)
        {
            if(flag_val[c-1]==flag_val[c]&& flag_val[c]>0)
            {
                count++;
                //                if(is_onece&&count>PSize/2)
                if(count>PSize/2)
                {
//                     if(_nonmax_supression)
//                     {
//                         core=coreScore<PSize>(flag_d,_threshold);
//                     }
//                     else
//                     {
                        iscorner=true;
//                     }
//                    is_onece=false;
                }
            }
            else
            {   
                count=1;
            }
        }
        if(_nonmax_supression && iscorner) {
            core=coreScore<PSize>(flag_d,_threshold);
        }
        //    }
    return iscorner;
}
template<int KERNEL_SIZE,typename SRC_T>
bool fast_nonmax(
        hls::Window<KERNEL_SIZE,KERNEL_SIZE,SRC_T>  & core_win
        )
{
#pragma HLS INLINE
    bool iscorner=true;
    for(int k=0;k<3;k++)
    {
        if(core_win.val[1][1]>core_win.val[0][k])
            iscorner=iscorner&&true;
        else
            iscorner=iscorner&&false;
    }
    for(int k=0;k<3;k++)
    {
        if(core_win.val[1][1]>core_win.val[2][k])
            iscorner=iscorner&&true;
        else
            iscorner=iscorner&&false;
    }
    if(core_win.val[1][1]>core_win.val[1][0]&&core_win.val[1][1]>core_win.val[1][2])
        iscorner=iscorner&&true;
    else
        iscorner=iscorner&&false;
    return iscorner;
}
//generate mask
template<int PSize,int KERNEL_SIZE,int SRC_T,int ROWS,int COLS>
void FAST_t_opr(
        hls::Mat<ROWS,COLS,SRC_T>    &_src,
        hls::Mat<ROWS,COLS,SRC_T>    &_mask,
        int                     _threshold,
        bool                    _nonmax_supression,
        int                     (&flag)[PSize][2]
        )
{
    hls::LineBuffer<KERNEL_SIZE-1,COLS,HLS_TNAME(SRC_T)>    k_buf;
    hls::LineBuffer<2,COLS+KERNEL_SIZE,ap_int<16> >         core_buf;
    hls::Window<3,3,ap_int<16> >                            core_win;
    hls::Window<KERNEL_SIZE,KERNEL_SIZE,HLS_TNAME(SRC_T)>       win;
    hls::Scalar<HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>             s;
    hls::Scalar<HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>             d;
    int rows= _src.rows;
    int cols= _src.cols;
    int kernel_half=KERNEL_SIZE/2;
    int flag_val[PSize+PSize/2+1];
    int flag_d[PSize+PSize/2+1];
#pragma HLS ARRAY_PARTITION variable=flag_val dim=0
#pragma HLS ARRAY_PARTITION variable=flag_d dim=0
    int index=0;
    int offset=KERNEL_SIZE/2;
    if(_nonmax_supression)
    {
        offset=offset+1;
    }
    for(HLS_SIZE_T i=0;i<rows+offset;i++)
    {
#pragma HLS LOOP_TRIPCOUNT MAX=ROWS
        for(HLS_SIZE_T j=0;j<cols+offset;j++)
        {
#pragma HLS DEPENDENCE ARRAY INTER FALSE
#pragma HLS LOOP_TRIPCOUNT MAX=COLS
#pragma HLS LOOP_FLATTEN off
#pragma HLS PIPELINE II=1
            d.val[0]=0;
            if(i<rows&&j<cols)
            {
                for(int r= 0;r<KERNEL_SIZE;r++)
                {
                    for(int c=0;c<KERNEL_SIZE-1;c++)
                    {
                        win.val[r][c]=win.val[r][c+1];//column left shift
                    }
                }
                win.val[0][KERNEL_SIZE-1]=k_buf.val[0][j];
                for(int buf_row= 1;buf_row< KERNEL_SIZE-1;buf_row++)
                {
                    win.val[buf_row][KERNEL_SIZE-1]=k_buf.val[buf_row][j];
                    k_buf.val[buf_row-1][j]=k_buf.val[buf_row][j];
                }
                //-------
                _src>>s;
                win.val[KERNEL_SIZE-1][KERNEL_SIZE-1]=s.val[0];
                k_buf.val[KERNEL_SIZE-2][j]=s.val[0];
            }
                //------core
                for(int r= 0;r<3;r++)
                {
                    for(int c=0;c<3-1;c++)
                    {
                        core_win.val[r][c]=core_win.val[r][c+1];//column left shift
                    }
                }
                core_win.val[0][3-1]=core_buf.val[0][j];
                for(int buf_row= 1;buf_row< 3-1;buf_row++)
                {
                    core_win.val[buf_row][3-1]=core_buf.val[buf_row][j];
                    core_buf.val[buf_row-1][j]=core_buf.val[buf_row][j];
                }
                int core=0;
                //output
                if(i>=KERNEL_SIZE-1&&j>=KERNEL_SIZE-1)
                {
                    //process
                    bool iscorner=fast_judge<PSize>(win,_threshold,flag_val,flag_d,flag,core,_nonmax_supression);
                    if(iscorner&&!_nonmax_supression)
                    {
                        d.val[0]=255;
                    }
                }
                if(i>=rows||j>=cols)
                {
                    core=0;
                }
                if(_nonmax_supression)
                {
                    core_win.val[3-1][3-1]=core;
                    core_buf.val[3-2][j]=core;
                    if(core_win.val[1][1]!=0&&i>=KERNEL_SIZE&&j>=KERNEL_SIZE)
                    {
                        bool iscorner=fast_nonmax(core_win);
                        if(iscorner)
                        {
                            d.val[0]=255;
                        }
                    }
                }
            
            if(i>=offset&&j>=offset)
            {
                _mask<<d;
            }
        }
    }
}
template<int SRC_T,int ROWS,int COLS>
void FASTX(
        hls::Mat<ROWS,COLS,SRC_T>    &_src,
        hls::Mat<ROWS,COLS,HLS_8UC1> &_mask,
        int    _threshold,
        bool   _nomax_supression
        )
{
#pragma HLS INLINE
    hls::Mat<ROWS,COLS,HLS_8UC1>    gray(ROWS,COLS);
    hls::CvtColor<HLS_BGR2GRAY>(_src,gray);
    int flag[16][2]={{3,0},{4,0},{5,1},{6,2},{6,3},{6,4},{5,5},{4,6},
        {3,6},{2,6},{1,5},{0,4},{0,3},{0,2},{1,1},{2,0}};
    FAST_t_opr<16,7>(gray,_mask,_threshold,_nomax_supression,flag);
}
template<int SRC_T,int MASK_T,int ROWS,int COLS>
void PaintMask(
        hls::Mat<ROWS,COLS,SRC_T>       &_src,
        hls::Mat<ROWS,COLS,MASK_T>      &_mask,
        hls::Mat<ROWS,COLS,SRC_T>       &_dst,
        hls::Scalar<HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)> _color
        )
{
    int rows= _src.rows;
    int cols= _src.cols;
    hls::Scalar<HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>      s;
    hls::Scalar<HLS_MAT_CN(SRC_T),HLS_TNAME(SRC_T)>      d;
    hls::Scalar<HLS_MAT_CN(MASK_T),HLS_TNAME(MASK_T)>    m;
    for(HLS_SIZE_T i=0;i<rows;i++)
    {
#pragma HLS LOOP_TRIPCOUNT MAX=ROWS
        for(HLS_SIZE_T j=0;j<cols;j++)
        {
#pragma HLS LOOP_TRIPCOUNT MAX=COLS
#pragma HLS LOOP_FLATTEN off
#pragma HLS PIPELINE II=1
            _src>> s;
            _mask>>m;
            if(m.val[0]!=0)
            {
                _dst<<_color;
            }
            else
            {
                _dst<<s;
            }
        }
    }
}
#endif 
