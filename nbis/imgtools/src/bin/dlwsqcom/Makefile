#*******************************************************************************
#
# License: 
# This software and/or related materials was developed at the National Institute
# of Standards and Technology (NIST) by employees of the Federal Government
# in the course of their official duties. Pursuant to title 17 Section 105
# of the United States Code, this software is not subject to copyright
# protection and is in the public domain. 
#
# This software and/or related materials have been determined to be not subject
# to the EAR (see Part 734.3 of the EAR for exact details) because it is
# a publicly available technology and software, and is freely distributed
# to any interested party with no licensing requirements.  Therefore, it is 
# permissible to distribute this software as a free download from the internet.
#
# Disclaimer: 
# This software and/or related materials was developed to promote biometric
# standards and biometric technology testing for the Federal Government
# in accordance with the USA PATRIOT Act and the Enhanced Border Security
# and Visa Entry Reform Act. Specific hardware and software products identified
# in this software were used in order to perform the software development.
# In no case does such identification imply recommendation or endorsement
# by the National Institute of Standards and Technology, nor does it imply that
# the products and equipment identified are necessarily the best available
# for the purpose.
#
# This software and/or related materials are provided "AS-IS" without warranty
# of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
# NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
# or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
# licensed product, however used. In no event shall NIST be liable for any
# damages and/or costs, including but not limited to incidental or consequential
# damages of any kind, including economic damage or injury to property and lost
# profits, regardless of whether NIST shall be advised, have reason to know,
# or in fact shall know of the possibility.
#
# By using this software, you agree to bear all risk relating to quality,
# use and performance of the software and/or related materials.  You agree
# to hold the Government harmless from any claim arising from your use
# of the software.
#
#*******************************************************************************

# SubTree:              /NBIS/Main/imgtools/src/bin/dlwsqcom
# Filename:             Makefile
# Integrators:          Kenneth Ko
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         08/20/2006
# Date Updated:         09/04/2008
# Date Updated:         01/06/2009 (Kenneth Ko) - add support for HPUX compile
#
# ******************************************************************************
#
# Makefile contains the variables to build binary - "dlwsqcom".
#
# ******************************************************************************
include ../../../p_rules.mak
#
PROGRAM	:= dlwsqcom
#
SRC	:= dlwsqcom.c
#
LIBS	= \
	$(EXPORTS_LIB_DIR)/libimage.a \
	$(EXPORTS_LIB_DIR)/libihead.a \
	$(EXPORTS_LIB_DIR)/libwsq.a \
	$(EXPORTS_LIB_DIR)/libjpegl.a \
	$(EXPORTS_LIB_DIR)/libfet.a \
	$(EXPORTS_LIB_DIR)/libioutil.a \
	$(EXPORTS_LIB_DIR)/libutil.a \
	$(EXPORTS_LIB_DIR)/liban2k.a
#
ifeq ($(NBIS_JASPER_FLAG),-D__NBIS_JASPER__)
LIBS	:= \
	$(LIBS) \
	$(EXPORTS_LIB_DIR)/libjasper.a 
endif
#
ifeq ($(NBIS_PNG_FLAG),-D__NBIS_PNG__)
LIBS	:= \
	$(LIBS) \
	$(EXPORTS_LIB_DIR)/libpng.a \
	$(EXPORTS_LIB_DIR)/libz.a
endif
#
EXT_INCS	:= -I$(EXPORTS_INC_DIR)
#
EXT_LIBS	:= -lm
#
include $(DIR_ROOT_BUILDUTIL)/bin.mak
