#*******************************************************************************
#
# License: 
# This software was developed at the National Institute of Standards and 
# Technology (NIST) by employees of the Federal Government in the course 
# of their official duties. Pursuant to title 17 Section 105 of the 
# United States Code, this software is not subject to copyright protection 
# and is in the public domain. NIST assumes no responsibility  whatsoever for 
# its use by other parties, and makes no guarantees, expressed or implied, 
# about its quality, reliability, or any other characteristic. 
#
# This software has been determined to be outside the scope of the EAR
# (see Part 734.3 of the EAR for exact details) as it has been created solely
# by employees of the U.S. Government; it is freely distributed with no
# licensing requirements; and it is considered public domain.  Therefore,
# it is permissible to distribute this software as a free download from the
# internet.
#
# Disclaimer: 
# This software was developed to promote biometric standards and biometric
# technology testing for the Federal Government in accordance with the USA
# PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
# Specific hardware and software products identified in this software were used
# in order to perform the software development.  In no case does such
# identification imply recommendation or endorsement by the National Institute
# of Standards and Technology, nor does it imply that the products and equipment
# identified are necessarily the best available for the purpose.  
#
#*******************************************************************************

# SubTree:              /NBIS/Main/buildutil
# Filename:             openjp2_libs.mak
# Integrators:          Kenneth Ko
#			John Grantham
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         05/15/2009 (Kenneth Ko)
# Date Updated:		10/22/2013 (Kenneth Ko)
#			05/08/2014 (John Grantham) - Updated for openjpeg2
#			07/30/2014 (John Grantham) - Updated to support 
#						     "out-of-source" build mode
#			08/05/2014 (John Grantham) - Added CYGWIN support
#			01/08/2015 (John Grantham) - Added -O2 and -w flags to
#						     optimize binaries and 
#						     suppress warnings
#			02/17/2015 (John Grantham) - Renamed "openjpeg2" to
#						     "openjp2"
#
# ******************************************************************************
#
# Makefile to loop through openjpeg libaray directories and call the next
# level Makefile.
#
# ******************************************************************************
bins:
	@(cd $(DIR_ROOT_BUILDUTIL) && $(MAKE) -f ./openjp2_bins.mak bin) || exit 1 
#
libs:
	@echo "Start: Compiling openjp2 library...."
	@(cd $(DIR_SRC_LIB)/openjp2/build && $(CHMOD) -R +w * && $(MAKE)) || exit 1
	@(cd $(DIR_SRC_LIB)/openjp2/build && ($(CP) ./bin/libopenjp2.a $(EXPORTS_LIB_DIR))) || exit 1
	@echo "End: Compiling openjp2 library."
#
config:
	@if [ "$(MSYS_FLAG)" = "-D__MSYS__" ]; then \
		echo "Building openjp2 library with MSYS..."; \
		(cd $(DIR_SRC_LIB)/openjp2 && mkdir -p build && cd build && cmake -DBUILD_THIRDPARTY:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -G "MSYS Makefiles" -D NBIS_ROOT="$(DIR_ROOT)" -DCMAKE_C_FLAGS="$(ARCH_FLAG) -O2 -w" ..) || exit 1; \
	elif [ "$(CYGWIN_FLAG)" = "-D__CYGWIN__" ]; then \
		echo "Building openjp2 library with CYGWIN..."; \
		(cd $(DIR_SRC_LIB)/openjp2 && mkdir -p build && cd build && cmake -DBUILD_THIRDPARTY:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -D NBIS_ROOT="$(DIR_ROOT)" -DCMAKE_C_FLAGS="$(ARCH_FLAG) -O2 -w" -DCMAKE_LEGACY_CYGWIN_WIN32=0 ..) || exit 1; \
	else \
		echo "Building openjp2 library..."; \
		(cd $(DIR_SRC_LIB)/openjp2 && mkdir -p build && cd build && cmake -DBUILD_THIRDPARTY:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -D NBIS_ROOT="$(DIR_ROOT)" -DCMAKE_C_FLAGS="$(ARCH_FLAG) -O2 -w" ..) || exit 1; \
	fi;
#
install:
	@(cd $(DIR_ROOT_BUILDUTIL) && $(MAKE) -f ./openjp2_bins.mak install) || exit 1
	@(cd $(DIR_SRC_LIB)/openjp2 && ($(CP) ./build/bin/libopenjp2.a $(INSTALL_ROOT_LIB_DIR))) || exit 1 
#
clean:
	@(cd $(DIR_SRC_LIB)/openjp2 && cd build && $(MAKE) clean && cd bin && $(RM) -rf *) || exit 1
#
catalog:
#
