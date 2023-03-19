#!/bin/sh
# ******************************************************************************
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
#
# ******************************************************************************
# Project:              NIST Fingerprint Software
# SubTree:              /NBIS/Main
# Filename:             setup.sh
# Integrators:          Kenneth Ko
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         08/20/2006
# Date Updated:         02/20/2007 (Kenneth Ko)
#                       10/23/2007 (Kenneth Ko)
#                       11/16/2007 (Kenneth Ko)
#                       01/31/2008 (Kenneth Ko)
#                       03/20/2008 (Kenneth Ko)
#                       03/27/2008 (Kenneth Ko)
#                       04/14/2008 (Kenneth Ko)
#                       09/03/2008 (Kenneth Ko)
#                       12/02/2008 (Kenneth Ko) - Fix to support 64-bit
#                       12/16/2008 (Kenneth Ko) - Add command line option for
#                                                 HPUX.
#                       12/17/2008 (Kenneth Ko) - Reorganize what packages to 
#                                                 use to build the software on
#                                                 HPUX.
#                       01/16/2009 (Greg Fiumara) - Switch standard library
#                                                   configuration to 
#                                                   __NBIS_<STDLIBS>__
#                       01/09/2009 (Greg Fiumara) - Add --64/--32 option to 
#                                                   force architecture on
#                                                   cross-compile capable 
#                                                   systems.
#                       05/02/2011 (Kenneth Ko) - Update licensing information.
#                                                 Add fPIC flag.
#                                                 Build PNG libaray as default 
#                                                 on MSYS.
#                       05/09/2012 (Kenneth Ko) - Remove 32-bit or 64-bit 
#                                                 architecture check.
#                                                 Default to build with fPIC.
#                                                 Remove --with-JASPER support.
#                                                 Add --without-OPENJPEG option
#                                                 to disable building OpenJPEG
#                                                 library.
#                                                 Check arch.mak exist or not 
#                                                 before delete.
#                       10/22/2013 (Kenneth Ko) - Bug fix for MSYS installation. 
#                       05/08/2014 (John Grantham) - Added support for openjpeg2
#                       08/05/2014 (John Grantham) - Added --CYGWIN option to
#                                                    support building openjpeg2,
#                                                    which requires passing a
#                                                    flag to CMake for CYGWIN
#                       02/17/2015 (John Grantham) - Renamed "openjpeg2" to
#                                                    "openjp2"
#                       02/24/2015 (Kenneth Ko)    - Renamed option
#                                                    "--without-openjpeg" to
#                                                    "--without-openjp2"
#
#                                                  
# ******************************************************************************
#
# This is a setup script. Run this prior to compilation.
#
# ******************************************************************************
# Check number of argument
if [ $# -lt 1 -o $# -gt 4 ] ; then 
	# ERROR
	echo "    For detail information on how to build NBIS, please consult the";
	echo "    INSTALLATION_GUIDE.";
	exit 1
fi

# Check the first argument is directory
if [ ! -d $1 ]; then
	# ERROR
	echo "Directory \"$1\" doesn't exist!";
	exit 1
else
	FINAL_INSTALLATION_DIR=$1
	INSTALL_DATA_DIR_STRING=$FINAL_INSTALLATION_DIR/nbis
	shift
fi

#--------------------------- Check and Set Flag --------------------------------
X11_FLAG=1
STDLIBS_FLAG=0
NBIS_JASPER_FLAG=0
NBIS_PNG_FLAG=1
NBIS_OPENJP2_FLAG=1
X86_64_FLAG=-1
TARGET_OS=0
MSYS_FLAG=0
FPIC_FLAG=0
CYGWIN_FLAG=0

while [ $# -ge 1 ]; do
	case $1 in
	--without-X11)
		X11_FLAG=0;;
	--STDLIBS)
		STDLIBS_FLAG=1
		NBIS_PNG_FLAG=0
		NBIS_OPENJP2_FLAG=0;;
	--without-OPENJP2)
		NBIS_OPENJP2_FLAG=0;;
	--64)         
		X86_64_FLAG=1;;
	--32)         
		X86_64_FLAG=0;;
	--SUPERDOME)
		TARGET_OS=1
		STDLIBS_FLAG=1
		NBIS_PNG_FLAG=0
		X11_FLAG=0;;
	--MSYS)
		MSYS_FLAG=1
		STDLIBS_FLAG=1
		X11_FLAG=0;;
	--CYGWIN)
		CYGWIN_FLAG=1
		STDLIBS_FLAG=1
		FPIC_FLAG=0
		X11_FLAG=0;;
	*)
		echo "No such option: $1"
		exit 1;;
	esac
	shift
done

#---------------------------------- X11 path -----------------------------------
OS_FLAG=0

if [ `uname` = "Linux" ]; then
	X11_INC="/usr"
        X11_LIB="/usr"
elif [ `uname` = "Darwin" ]; then  
	X11_INC="/usr/X11R6/include"
	X11_LIB="/usr/X11R6/lib"
	OS_FLAG=1
elif [ `uname` = "CYGWIN_NT-5.1" ]; then
	X11_INC="/usr/X11R6/include"
	X11_LIB="/usr/X11R6/lib"
else
	X11_INC="/usr/X11R6"
        X11_LIB="/usr/X11R6"
fi

if [ $X11_FLAG -eq 1 ] ; then
	if [ ! -d $X11_INC ]; then
		echo "Directory $X11_INC doesn't exist!";
		echo "Please update variable X11_INC in setup.sh, if you wish to compile with X11."
		exit 1
	fi
	if [ ! -d $X11_LIB ]; then
		echo "Library $X11_LIB not find!";
		echo "Please update variable X11_INC in setup.sh, if you wish to compile with X11."
		exit 1
	fi
fi

#------------------------------- Set Current Path --------------------------------
# Set Path
if [ $MSYS_FLAG -eq 1 ]; then
        MAIN_DIR=$PWD
        cd ..
        NBIS_DIR=`dirname $PWD;`
else
        MAIN_DIR=$PWD
        cd ..
        NBIS_DIR=$PWD
fi

cd $MAIN_DIR

#---------------------------------- Check Endian ---------------------------------
# Detect target machine OS
rulesFile="rules.mak.src"

# Detect target machine's endian
if [ $MSYS_FLAG -eq 1 ]; then
	RM=rm
	CC=gcc
	CP=cp
else
	RM=`which rm`
	CC=gcc
	CP=`which cp`
fi

endianFile=./endian.out

if [ -e ${endianFile} ]; then
	`${RM} ${endianFile}`
fi

`${CC} am_big_endian.c -o am_big_endian`
`./am_big_endian > ${endianFile}`

if [ ! -e ${endianFile} ]; then
	echo "Failed Setup - Cannot find file - ${endianFile}"
	exit 1
fi

if [ ! -s ${endianFile} ]; then
	echo "Failed Setup - Cannot determine target machine endianness!"
	exit 1  
fi

endian=`head -n 1 ${endianFile}`

# See if the compiler supports generating position independent code
FPIC_FLAG=1

#----------------------------- Use "sed" to Modify ------------------------------
# Set PACKAGES varabile in rules.mak.src which depend on the export control
# directory(s) exist or not.
IJG='ijg'
JPEG2K='jpeg2k'
NBIS_PNG='png'
OPENJP2='openjp2' 
REG_LIBS='commonnbis an2k bozorth3 imgtools mindtct nfseg nfiq pcasys'

if [ $NBIS_JASPER_FLAG -eq 1 -a $NBIS_PNG_FLAG -eq 0 ]; then
	IMAGE_LIBS="${IJG} ${JPEG2K}"
elif [ $NBIS_JASPER_FLAG -eq 0 -a $NBIS_PNG_FLAG -eq 1 ]; then
	IMAGE_LIBS="${IJG} ${NBIS_PNG}"
elif [ $NBIS_JASPER_FLAG -eq 1 -a $NBIS_PNG_FLAG -eq 1 ]; then
	IMAGE_LIBS="${IJG} ${JPEG2K} ${NBIS_PNG}"
elif [ $NBIS_JASPER_FLAG -eq 0 -a $NBIS_PNG_FLAG -eq 0 ]; then
	IMAGE_LIBS="${IJG}"
fi

if [ $NBIS_OPENJP2_FLAG -eq 1 ]; then
	IMAGE_LIBS="${IMAGE_LIBS} ${OPENJP2}"
fi

PACKAGES="${IMAGE_LIBS} ${REG_LIBS}"

if [ $MSYS_FLAG -eq 1 ]; then 
	cat rules_msys.mak.src | sed 's,SED_PACKAGES_STRING,'"$PACKAGES"',' > rules.mak.temp1
elif [ $TARGET_OS -eq 1 ]; then 
	cat rules_superdome.mak.src | sed 's,SED_PACKAGES_STRING,'"$PACKAGES"',' > rules.mak.temp1
else
	cat rules.mak.src | sed 's,SED_PACKAGES_STRING,'"$PACKAGES"',' > rules.mak.temp1
fi

# Use 'sed' command to modify all the necessary files
cat rules.mak.temp1 | sed 's,SED_DIR_MAIN,'$MAIN_DIR',' > rules.mak.temp2
cat rules.mak.temp2 | sed 's,SED_FINAL_INSTALLATION_STRING,'$FINAL_INSTALLATION_DIR',' > rules.mak.temp3
cat rules.mak.temp3 | sed 's,SED_X11_FLAG,'$X11_FLAG',' > rules.mak.temp4
cat rules.mak.temp4 | sed 's,SED_X11_INC,'$X11_INC',' > rules.mak.temp5
cat rules.mak.temp5 | sed 's,SED_X11_LIB,'$X11_LIB',' > rules.mak.temp6

# Use 'sed' command to modify ENDIAN_FLAG
if [ $endian -eq 1 ]; then
	ENDIAN_FLAG="-D__NBISLE__"
elif [ $endian -eq 0 ]; then
	ENDIAN_FLAG="";
fi
cat rules.mak.temp6 | sed 's,SED_ENDIAN_FLAG,'$ENDIAN_FLAG',' > rules.mak.temp7

# Use 'sed' command to modify J_FLAG
if [ $NBIS_JASPER_FLAG -eq 1 ]; then
	J_FLAG="-D__NBIS_JASPER__"
	cat rules.mak.temp7 | sed 's,SED_NBIS_JASPER_FLAG,'$J_FLAG',' > rules.mak.temp8
elif [ $NBIS_JASPER_FLAG -eq 0 ]; then
	J_FLAG="";
	cat rules.mak.temp7 | sed 's,SED_NBIS_JASPER_FLAG,'$J_FLAG',' > rules.mak.temp8
fi

# Use 'sed' command to modify ARCH_FLAG
if [ -e arch.mak ]; then
	`${RM} arch.mak`
fi

if [ $X86_64_FLAG -eq -1 ]; then
	ARCH_FLAG=""
	echo "ARCH_FLAG = " >> arch.mak
elif [ $X86_64_FLAG -eq 1 ]; then
	ARCH_FLAG="-m64"
	echo "ARCH_FLAG = -m64" >> arch.mak
elif [ $X86_64_FLAG -eq 0 ]; then
	ARCH_FLAG="-m32"
	echo "ARCH_FLAG = -m32" >> arch.mak
fi
if [ $FPIC_FLAG -eq 1 ] && [ $CYGWIN_FLAG -eq 0 ] && [ $MSYS_FLAG -eq 0 ]; then
	ARCH_FLAG="$ARCH_FLAG -fPIC"
fi
echo "ARCH_FLAG = $ARCH_FLAG" >> arch.mak
cat rules.mak.temp8 | sed "s,SED_ARCH_FLAG,$ARCH_FLAG," > rules.mak.temp9

# Use 'sed' command to modify P_FLAG
if [ $NBIS_PNG_FLAG -eq 1 ]; then
        P_FLAG="-D__NBIS_PNG__"
        cat rules.mak.temp9 | sed 's,SED_NBIS_PNG_FLAG,'$P_FLAG',' > rules.mak.temp10
elif [ $NBIS_PNG_FLAG -eq 0 ]; then
        P_FLAG="";
        cat rules.mak.temp9 | sed 's,SED_NBIS_PNG_FLAG,'$P_FLAG',' > rules.mak.temp10
fi

# Use 'sed' commdand to modify MSYS_FLAG
if [ $MSYS_FLAG -eq 1 ]; then
	M_FLAG="-D__MSYS__"
	cat rules.mak.temp10 | sed 's,SED_MSYS_FLAG,'$M_FLAG',' > rules.mak.temp11
elif [ $MSYS_FLAG -eq 0 ]; then
	M_FLAG=""
	cat rules.mak.temp10 | sed 's,SED_MSYS_FLAG,'$M_FLAG',' > rules.mak.temp11
fi

# Use 'sed' commdand to modify OS_FLAG
if [ $OS_FLAG -eq 1 ]; then
	O_FLAG="DARWIN"
	cat rules.mak.temp11 | sed 's,SED_OS_FLAG,'$O_FLAG',' > rules.mak.temp12
elif [ $OS_FLAG -eq 0 ]; then
	O_FLAG=""
	cat rules.mak.temp11 | sed 's,SED_OS_FLAG,'$O_FLAG',' > rules.mak.temp12
fi

# Use 'sed' commdand to modify CYGWIN_FLAG
if [ $CYGWIN_FLAG -eq 1 ]; then
	CYG_FLAG="-D__CYGWIN__"
	cat rules.mak.temp12 | sed 's,SED_CYGWIN_FLAG,'$CYG_FLAG',' > rules.mak.temp13
elif [ $CYGWIN_FLAG -eq 0 ]; then
	CYG_FLAG=""
	cat rules.mak.temp12 | sed 's,SED_CYGWIN_FLAG,'$CYG_FLAG',' > rules.mak.temp13
fi


# Use 'sed' commdand to modify NBIS_OPENJP2_FLAG
if [ $NBIS_OPENJP2_FLAG -eq 1 ]; then
	O_FLAG="-D__NBIS_OPENJP2__"
	cat rules.mak.temp13 | sed 's,SED_NBIS_OPENJP2_FLAG,'$O_FLAG',' > rules.mak
elif [ $NBIS_OPENJP2_FLAG -eq 0 ]; then
	O_FLAG="";
	cat rules.mak.temp13 | sed 's,SED_NBIS_OPENJP2_FLAG,'$O_FLAG',' > rules.mak
fi

rm -f rules.mak.temp1 rules.mak.temp2 rules.mak.temp3 rules.mak.temp4 rules.mak.temp5 rules.mak.temp6 rules.mak.temp7 rules.mak.temp8 rules.mak.temp9 rules.mak.temp10 rules.mak.temp11 rules.mak.temp12 rules.mak.temp13

cd ${MAIN_DIR}/an2k/include
sed 's,SED_INSTALL_DATA_DIR_STRING,'$INSTALL_DATA_DIR_STRING',' < an2k.h.src > an2k.h

cd ${MAIN_DIR}/pcasys/include
cat little.h.src | sed 's,SED_INSTALL_DIR_STRING,'$MAIN_DIR',' > little.h.temp1
cat little.h.temp1 | sed 's,SED_INSTALL_DATA_DIR_STRING,'$INSTALL_DATA_DIR_STRING',' > little.h.temp2
cat little.h.temp2 | sed 's,SED_INSTALL_NBIS_DIR_STRING,'$NBIS_DIR',' > little.h
rm -f little.h.temp1 little.h.temp2

cd ${MAIN_DIR}/pcasys/src/bin/optrws
cat optrws.c.src | sed 's,SED_INSTALL_BIN_DIR_STRING,'$INSTALL_BIN_DIR_STRING',' > optrws.c

if [ $TARGET_OS -eq 1 ]; then
	cd ${MAIN_DIR}/an2k/src/lib/an2k
	${CP} select.c.superdome select.c
else
	cd ${MAIN_DIR}/an2k/src/lib/an2k
	${CP} select.c.linux select.c
fi

chmod +w ${MAIN_DIR}/jpeg2k/src/lib/jasper/configure
chmod +w ${MAIN_DIR}/jpeg2k/src/lib/jasper/aclocal.m4
chmod +w ${MAIN_DIR}//an2k/src/lib/an2k/select.c
