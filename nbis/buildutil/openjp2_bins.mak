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

# Filename:             openjp2_bins.mak
# Integrators:          Kenneth Ko
#			John Grantham
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         05/15/2009 (Kenneth Ko)
# Date Updated:         04/18/2011 (Kenneth Ko)
#			05/08/2014 (John Granhtam) - Updated for openjpeg2
#			07/30/2014 (John Grantham) - Updated to support 
#						     "out-of-source" build mode
#			02/17/2015 (John Grantham) - Renamed "openjpeg2" to
#						     "openjp2"
#
# ******************************************************************************
#
# Makefile to loop through openjpeg binary directories and call the next
# level Makefile.
#
# ******************************************************************************
include ../openjp2/p_rules.mak
#
bin:
#
install:
	@(cd $(DIR_SRC_LIB)/openjp2 && $(CP) ./build/bin/opj_decompress $(INSTALL_ROOT_BIN_DIR)/djp2k) || exit 1
	@(cd $(DIR_SRC_LIB)/openjp2 && $(CP) ./build/bin/opj_compress $(INSTALL_ROOT_BIN_DIR)/cjp2k) || exit 1
#
clean:
#

