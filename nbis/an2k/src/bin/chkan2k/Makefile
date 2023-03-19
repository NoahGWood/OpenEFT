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
# Project:              NIST Fingerprint Software
# SubTree:              /NBIS/Main/an2k/src/bin/chkan2k
# Filename:             Makefile
# Integrators:          Kenneth Ko
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:	        2009/01/12 by Joseph Konczal
#			            copied from an2k2txt and modified for
#		                chkan2k by global string substitution
#                       08/19/2014 (Kenneth Ko) - add OpenJPEG2 support
#                       02/25/2015 (Kenneth Ko) - Renamed OPENJPEG to OPENJP2
#
# ******************************************************************************
#
# Makefile contains the variables to build binary - "chkan2k".
#
# ******************************************************************************
include ../../../p_rules.mak
#
PROGRAM	:= chkan2k
#
SRC	:= chkan2k.c config.c logger.c chkfile.c combos.c
#
LIBS	= $(EXPORTS_LIB_DIR)/liban2k.a \
	$(EXPORTS_LIB_DIR)/libimage.a \
	$(EXPORTS_LIB_DIR)/libihead.a \
	$(EXPORTS_LIB_DIR)/libwsq.a \
	$(EXPORTS_LIB_DIR)/libjpegl.a \
	$(EXPORTS_LIB_DIR)/libjpegb.a \
	$(EXPORTS_LIB_DIR)/libfet.a \
	$(EXPORTS_LIB_DIR)/libioutil.a \
	$(EXPORTS_LIB_DIR)/libutil.a
#
ifeq ($(NBIS_JASPER_FLAG),-D__NBIS_JASPER__)
LIBS	:= \
	$(LIBS) \
	$(EXPORTS_LIB_DIR)/libjasper.a
endif
#
ifeq ($(NBIS_OPENJP2_FLAG),-D__NBIS_OPENJP2__)
LIBS	:= \
	$(LIBS) \
	$(EXPORTS_LIB_DIR)/libopenjp2.a 
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

#
unit_test: test_config

TC_OBJFILES:=$(patsubst %chkan2k.o,,$(patsubst %config.o,,$(OBJFILES)))

test_config: config.c $(TC_OBJFILES)
	$(CC) -DUNIT_TEST $(CFLAGS) -I$(DIR_INC) $(EXT_INCS) -o $@ $< $(TC_OBJFILES) $(LIBS) $(EXT_LIBS)
