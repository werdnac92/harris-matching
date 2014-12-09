#***************************************************************************
#
#   © Copyright 2013 Xilinx, Inc. All rights reserved. 
#
#   This file contains confidential and proprietary information of Xilinx,
#   Inc. and is protected under U.S. and international copyright and other
#   intellectual property laws. 
#
#   DISCLAIMER
#   This disclaimer is not a license and does not grant any rights to the
#   materials distributed herewith. Except as otherwise provided in a valid
#   license issued to you by Xilinx, and to the maximum extent permitted by
#   applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH
#   ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, 
#   EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES
#   OF MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR 
#   PURPOSE; and (2) Xilinx shall not be liable (whether in contract or 
#   tort, including negligence, or under any other theory of liability)
#   for any loss or damage of any kind or nature related to, arising under
#   or in connection with these materials, including for any direct, or any
#   indirect, special, incidental, or consequential loss or damage (including
#   loss of data, profits, goodwill, or any type of loss or damage suffered 
#   as a result of any action brought by a third party) even if such damage
#   or loss was reasonably foreseeable or Xilinx had been advised of the 
#   possibility of the same. 
#  
#   CRITICAL APPLICATIONS 
#   Xilinx products are not designed or intended to be fail-safe, or for use
#   in any application requiring fail-safe performance, such as life-support
#   or safety devices or systems, Class III medical devices, nuclear facilities,
#   applications related to the deployment of airbags, or any other applications
#   that could lead to death, personal injury, or severe property or environmental
#   damage (individually and collectively, "Critical Applications"). Customer
#   assumes the sole risk and liability of any use of Xilinx products in Critical
#   Applications, subject only to applicable laws and regulations governing 
#   limitations on product liability. 
#   
#   THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT 
#   ALL TIMES.
#
#***************************************************************************/

THIS_DIR := $(shell pwd)
DESIGN := $(shell basename $(THIS_DIR))
TOP_DIR := $(shell dirname $(shell dirname $(THIS_DIR)))

RM := rm -rf

KNAME := $(shell uname -s)
HNAME := unknown
ifeq ($(shell uname -m), *86)
    HNAME := x86
else
    HNAME := x86_64
endif

ARCH := unknown
ifeq ($(HNAME)_$(KNAME), x86_64_Linux)
    ARCH := x86_64_Linux
else
    ifeq ($(HNAME)_$(KNAME), x86_Linux)
        ARCH := x86_Linux
    else
        ARCH := x86_Windows
    endif
endif

OPENCV_VERSION :=
ifeq ($(ARCH), x86_Windows)
    OPENCV_VERSION := 243
endif

OPENCV_INSTALL := $(TOP_DIR)/opencv_install
OPENCV_LIB := $(OPENCV_INSTALL)/$(ARCH)/lib
ARM_OPENCV_LIB := $(OPENCV_INSTALL)/arm_Linux/lib

APP_NAME := video_library_cmd

HLS_TCL := run.tcl
HLS_PRJ := prj
EDK_PRJ := xps_proj
BOOT_DIR := boot_image
SD_DIR := sd_image

HLS := $(shell vivado_hls -r)
AUTOESL_PATH := $(HLS)/include
AUTOESL_TECH := $(HLS)/common/technology/autopilot

CXX := g++
ifneq ($(ARCH), x86_Windows)
    CXX := $(HLS)/Linux_$(HNAME)/tools/gcc/bin/g++
endif

CXX_LIB :=
ifeq ($(ARCH), x86_64_Linux)
    CXX_LIB := $(HLS)/Linux_$(HNAME)/tools/gcc/lib64
else
    CXX_LIB := $(HLS)/Linux_$(HNAME)/tools/gcc/lib
endif

LIBS := -lpthread -lopencv_core$(OPENCV_VERSION) -lopencv_imgproc$(OPENCV_VERSION) -lopencv_highgui$(OPENCV_VERSION) -lopencv_features2d$(OPENCV_VERSION) -lopencv_flann$(OPENCV_VERSION)
ARM_LIBS := -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_features2d -lopencv_flann

#LFLAGS += `pkg-config --libs opencv`
#CFLAGS += `pkg-config --cflags opencv`

C_SRCS := $(wildcard ../share/*.c)

CPP_SRCS := $(wildcard ./*.cpp)

DESIGN_FILES := top.cpp

DESIGN_OBJS := $(CPP_SRCS:.cpp=.do)

OBJS := $(C_SRCS:.c=.o) top.o opencv_top.o

BMPS := result_1080p.bmp result_1080p_golden.bmp

C_HEADERS := $(wildcard ../share/*.h)

CPP_HEADERS := $(wildcard ./*.h)

%.cpp: $(CPP_HEADERS)

../share/%.c: $(C_HEADERS)

%.do: ./%.cpp $(CPP_HEADERS)
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ compiler'
	$(CXX) -O3 -I$(AUTOESL_PATH) -I$(AUTOESL_TECH) -I$(OPENCV_INSTALL)/$(ARCH)/include/opencv -I$(OPENCV_INSTALL)/$(ARCH)/include -c -fmessage-length=0 -fpermissive -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

../share/%.o: ../share/%.c $(C_HEADERS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Linux gcc compiler'
	arm-xilinx-linux-gnueabi-g++ -O3 -I$(THIS_DIR) -I$(OPENCV_INSTALL)/arm_Linux/include/opencv -I$(OPENCV_INSTALL)/arm_Linux/include -I$(AUTOESL_PATH) -I$(AUTOESL_TECH) -c -fmessage-length=0 -fpermissive -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ./%.cpp $(CPP_HEADERS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Linux gcc compiler'
	arm-xilinx-linux-gnueabi-g++ -O3 -I$(OPENCV_INSTALL)/arm_Linux/include/opencv -I$(OPENCV_INSTALL)/arm_Linux/include -I$(AUTOESL_PATH) -I$(AUTOESL_TECH) -c -fmessage-length=0 -fpermissive -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

