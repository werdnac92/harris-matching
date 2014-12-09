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

include ../configure.mk

# All Target
all: $(APP_NAME) $(BOOT_DIR)/BOOT.bin
	@cp -r $(TOP_DIR)/sd_image .; \
        chmod -R a+w $(SD_DIR); \
        cp -f $(APP_NAME) $(SD_DIR)/$(APP_NAME); \
        cp -f $(BOOT_DIR)/BOOT.bin $(SD_DIR)/BOOT.bin; \
        echo "New sd_image is ready."

$(APP_NAME): $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linux gcc linker'
	arm-xilinx-linux-gnueabi-g++ -L$(ARM_OPENCV_LIB) -o"$(APP_NAME)" $(OBJS) $(USER_OBJS) $(ARM_LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(APP_NAME).size: $(APP_NAME)
	@echo 'Invoking: ARM Linux Print Size'
	arm-xilinx-linux-gnueabi-size $(APP_NAME) |tee "$(APP_NAME).size"
	@echo 'Finished building: $@'
	@echo ' '

$(DESIGN): $(DESIGN_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ linker'
	$(CXX) -L$(OPENCV_LIB) -L$(CXX_LIB) $(LIBS) -o"$(DESIGN)" $(DESIGN_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

elf: $(APP_NAME) $(APP_NAME).size

sim: $(DESIGN)
	@export LD_LIBRARY_PATH=$(OPENCV_LIB):$(CXX_LIB):$(LD_LIBRARY_PATH); \
	./$(DESIGN)

core: $(DESIGN_FILES) 
	@vivado_hls $(HLS_TCL); 

$(HLS_PRJ)/solution1/impl/pcores/image_filter_top_v1_04_a: core

bitstream: $(HLS_PRJ)/solution1/impl/pcores/image_filter_top_v1_04_a
	@cp -r $(TOP_DIR)/hw/xps_proj .; \
        chmod -R a+w $(EDK_PRJ); \
        cp -rf $(HLS_PRJ)/solution1/impl/pcores/image_filter_top_v1_04_a $(EDK_PRJ)/pcores/; \
			rm -rf $(EDK_PRJ)/implementation/filter_engine*; \
			rm -rf $(EDK_PRJ)/implementation/cache/*filter_engine*; \
        cd $(EDK_PRJ); \
        echo 'run bits' | xps -nw system.xmp

$(EDK_PRJ)/implementation/system.bit: bitstream

boot: $(EDK_PRJ)/implementation/system.bit
	@cp -r $(TOP_DIR)/boot_image .; \
        chmod -R a+w $(BOOT_DIR); \
        cp $(EDK_PRJ)/implementation/system.bit $(BOOT_DIR)/system.bit; \
        cd $(BOOT_DIR); \
        ./bootgen.sh
         
$(BOOT_DIR)/BOOT.bin: boot

help:
	@echo 'Makefile targets of Video Library Reference Design:'; \
	echo '  make elf        -- build sw (video_library_cmd)'; \
	echo '  make sim        -- build and run csim test'; \
	echo '  make core       -- synthesis and export pcore'; \
	echo '  make bitstream  -- generate bitstream (system.bit)'; \
	echo '  make boot       -- generate boot image (BOOT.bin)'; \
	echo '  make all        -- build sw and hw, generate sd_image'; \
	echo '  make clean      -- clean generated files of sw and test'; \
	echo '  make allclean   -- clean all generated files'; \
	echo '  make help       -- give this help'; \
	echo ' '

clean:
	-$(RM) $(OBJS) $(DESIGN_OBJS) $(BMPS) *.log $(DESIGN)* $(APP_NAME)*
	-@echo ' '

allclean:
	-$(RM) $(OBJS) $(DESIGN_OBJS) $(BMPS) *.log $(DESIGN)* $(APP_NAME)* $(HLS_PRJ) $(EDK_PRJ) $(BOOT_DIR) $(SD_DIR) 
	-@echo ' '

.PHONY: all clean dependents allclean help
.SECONDARY:

