ECE 5775 Final Project
Andrew Chien (amc379), Kevin Lin (kl524), Deyu Liu (dl487)

Our algorithm takes an input image and performs Harris corner detection to 
pick out all the corners of the image. The Harris algorithm also computes an
angle of how much to rotate features. The features are then picked out of the
original image (a 7x7 square surrounding the center of the feature) and rotated,
and then stored in a MOPS format (a descriptor for the feature). The algorithm
then goes through all the MOPS between two pictures and compares them, marking
them similar if their similarity passes a certain threshold. 

Currently, we apply this to four quadrents of an image; the test.cpp program
reports how many similar feature pairs were found when comparing a quadrent
of "input1.bmp" with a quadrent of "input4.bmp". Thus if we take the lower-right
quadrent of input1.bmp and paste it in the upper-left quadrent of input4.bmp,
the algorithm should report a high number of matches between Q3 of input1.bmp
and Q0 of input4.bmp (and few matches between Q3 of input1.bmp and Q2 of
input4.bmp, for instance).

File Structure:
  test.cpp - all the code put together, to be run in simulation
  top.cpp - the code to be synthesized on the FPGA

Running Simulation:
  cd sw/seam/
  . ./xil-setup
  make sim

HLS Synthesis:
  cd sw/seam/
  . ./xil-setup
  vivado_hls -f run.tcl

  This generates a project file in sw/seam/prj, with generated verilog files
  in sw/seam/prj/solution1/syn/verilog/

Importing into ISE:
  cd sw/seam
  . ./xil-setup
  cd ../../
  cp /classes/ece5775/xilinx/zedboard/xillinux-eval-zedboard-1.1-gold.tgz .
  tar -vzxf xillinux-eval-zedboard-1.1-gold.tgz
  cd xillinux-eval-zedboard-1.1/verilog
  ise xillydemo.xise &
  
  in ISE: Project->Add Source. Choose all files (including subfolders) under sw/seam/prj/solution1/syn/verilog/*
