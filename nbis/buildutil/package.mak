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
# Filename:             package.mak
# Integrators:          Kenneth Ko
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         08/20/2006
# Date Updated:         03/27/2007
#                       11/27/2007
#                       02/25/2015
#
# ******************************************************************************
#
# Top-level make file to build package source code (libraries and binaries).
#
# ******************************************************************************
#
# config -- check to see if "nfis" have been installed by verifying
#           that all DIR_INSTALL directories are present.
#
# it -- Create a "obj" directory and compile/generate all the .o, .a
#       and executables under the "obj" directory.
#
# libs -- Compile/generate all .a under the "obj" directory.
#
# bins -- Compile/generate all .o under the "obj" directory.
#
# install -- Create "bin" and "lib" directories and install all the
#            .a and and executables from "obj" directory.
#
# clean -- cleanup all .o, .a and executables.
#
# catalog -- Create catalog file for the package libraries and
#            applications.
#
# help -- print help manual.
#
# ******************************************************************************
include ./p_rules.mak
#
# ******************************************************************************
# Target:   all:
# Target to make all libraries and executables.
#
# ******************************************************************************
all: \
	config \
	it \
	install
#
# ******************************************************************************
#
# Target: config:
# Target to verify all required project directories are present.
#
# ******************************************************************************
config:
	@echo "Start: Checking \"$(PACKAGE)\" directory structure...."
	@for dir in $(BASE_DIR) $(DIR_INC); do \
		echo "Verifying directory - \"$$dir\":"; \
		if [ ! -d $$dir ]; then \
			echo "[FAILED]"; \
			echo "Directory: \"$$dir\" doesn't exist!"; \
			echo "Please check out the missing directory \"$$dir\"!"; \
			date; \
			exit 2; \
		else \
			echo "[PASSED]"; \
		fi; \
	done
	@for dir in $(OBJ_BASE_DIR) $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR); do \
		status=0; \
		if [ ! -d $$dir ]; then \
			echo "Creating \"$$dir\" directory...."; \
			status=1; \
			$(MKDIR) $$dir; \
			echo "[PASSED]"; \
		fi; \
		if [ -d $$dir ]; then \
			if [ $$status = 0 ]; then \
				echo "Directory \"$$dir\" already exist:"; \
				echo "[PASSED]"; \
			fi; \
		else \
			echo "[FAILED]"; \
			exit 2; \
		fi; \
	done
	@(cd $(DIR_SRC) && $(MAKE) config) || exit 
	@echo "End: Checking \"$(PACKAGE)\" directory structure"
#
# ******************************************************************************
#
# Target: it:
# Target to make all the .o, .a and executables in obj directory.
#
# ******************************************************************************
it: libs bins
#
# ******************************************************************************
#
# Target: cpheaders:
# Target to copy all header files to $(EXPORTS_INC_DIR).
#
# ******************************************************************************
#
cpheaders:
	@if [ $(PACKAGE) = "ijg" ]; then \
		echo "Start: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"...."; \
		echo "$(CP) -Rpf $(DIR_SRC)/lib/jpegb/*.h $(EXPORTS_INC_DIR)"; \
		$(CP) -Rpf $(DIR_SRC)/lib/jpegb/*.h $(EXPORTS_INC_DIR); \
		echo "End: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"."; \
	elif [ $(PACKAGE) = "jpeg2k" ]; then \
		echo "Start: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"...."; \
		echo "$(CP) -Rpf $(DIR_SRC)/lib/jpeg2k/*.h $(EXPORTS_INC_DIR)"; \
		$(CP) -Rpf $(DIR_SRC)/lib/jasper/src/libjasper/include/jasper $(EXPORTS_INC_DIR); \
		$(RM) $(EXPORTS_INC_DIR)/jasper/Makefile*; \
		echo "End: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"."; \
	elif [ $(PACKAGE) = "png" ]; then \
		echo "Start: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"...."; \
		echo "$(CP) -Rpf $(DIR_SRC)/lib/png/*.h $(EXPORTS_INC_DIR)"; \
		$(CP) -Rpf $(DIR_SRC)/lib/png/*.h $(EXPORTS_INC_DIR); \
		echo "End: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"."; \
		echo "Start: Copying zlib's header files to \"$(EXPORTS_INC_DIR)\"...."; \
		echo "$(CP) -Rpf $(DIR_SRC)/lib/zlib/*.h $(EXPORTS_INC_DIR)"; \
		$(CP) -Rpf $(DIR_SRC)/lib/zlib/*.h $(EXPORTS_INC_DIR); \
		echo "End: Copying zlib's header files to \"$(EXPORTS_INC_DIR)\"."; \
	elif [ $(PACKAGE) = "openjp2" ]; then \
		echo "Start: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"...."; \
		$(MKDIR) $(EXPORTS_INC_DIR)/openjp2; \
		echo "$(CP) -Rpf $(DIR_SRC)/lib/openjp2/src/lib/openjp2/*.h $(EXPORTS_INC_DIR)/openjp2"; \
		$(CP) -Rpf $(DIR_SRC)/lib/openjp2/src/lib/openjp2/*.h $(EXPORTS_INC_DIR)/openjp2; \
        echo "$(CP) -Rpf $(DIR_SRC)/lib/openjp2/build/src/lib/openjp2/*.h $(EXPORTS_INC_DIR)/openjp2"; \
        $(CP) -Rpf $(DIR_SRC)/lib/openjp2/build/src/lib/openjp2/*.h $(EXPORTS_INC_DIR)/openjp2; \
	else \
		echo "Start: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"...."; \
		echo "$(CP) -Rpf $(DIR_INC)/* $(EXPORTS_INC_DIR)"; \
		$(CP) -Rpf $(DIR_INC)/* $(EXPORTS_INC_DIR); \
		echo "End: Copying $(PACKAGE)'s header files to \"$(EXPORTS_INC_DIR)\"." ; \
	fi
	@if [ $(PACKAGE) = "pcasys" ]; then \
		$(RM) $(EXPORTS_INC_DIR)/little.h.src; \
	fi
#
# ******************************************************************************
#
# Target: libs:
# Target to make all the .a in obj directory.
#
# ******************************************************************************
#
libs: cpheaders
	@if [ $(PACKAGE) = "ijg" ]; then \
		echo "Start: Compiling $(PACKAGE)'s libraries and binaries...."; \
		(cd $(DIR_SRC) && $(MAKE) libs) || exit 1; \
		echo "End: Compiling $(PACKAGE) libraries and binaries."; \
	elif [ $(PACKAGE) = "jpeg2k" ]; then \
		echo "Start: Compiling $(PACKAGE)'s libraries and binaries...."; \
		(cd $(DIR_SRC) && $(MAKE) libs) || exit 1; \
		echo "End: Compiling $(PACKAGE) libraries and binaries."; \
	elif [ $(PACKAGE) = "png" ]; then \
		echo "Start: Compiling $(PACKAGE)'s libraries and binaries...."; \
		(cd $(DIR_SRC) && $(MAKE) libs) || exit 1; \
		echo "End: Compiling $(PACKAGE) libraries and binaries."; \
	elif [ $(PACKAGE) = "openjp2" ]; then \
		echo "Start: Compiling $(PACKAGE)'s libraries and binaries...."; \
		(cd $(DIR_SRC) && $(MAKE) libs) || exit 1; \
		echo "End: Compiling $(PACKAGE) libraries and binaries."; \
	else \
		echo "Start: Compiling $(PACKAGE)'s libraries...."; \
		(cd $(DIR_SRC) && $(MAKE) libs) || exit 1; \
		echo "End: Compiling $(PACKAGE) libraries."; \
	fi
#
# ******************************************************************************
#
# Target: bins:
# Target to make all the .o in obj directory.
#
# ******************************************************************************
bins:
	@if [ $(PACKAGE) != "ijg" -a $(PACKAGE) != "jpeg2k" -a $(PACKAGE) != "png" ]; then \
		echo "Start: Compiling $(PACKAGE)'s binaries...."; \
		(cd $(DIR_SRC) && $(MAKE) bins) || exit 1; \
		echo "End: Compiling $(PACKAGE)'s binaries."; \
	fi
#
# ******************************************************************************
#
# Target: install
# Target to install the the .a in the lib and executables in the bin
# directory.
#
# ******************************************************************************
install:
	@echo "Start: Installing $(PACKAGE)'s binaries and libraries...."
	@(cd $(DIR_SRC) && $(MAKE) install) || exit 1
	@echo "End: Installing $(PACKAGE)'s binaries and libraries."
#
# ******************************************************************************
#
# Target: clean
# Target to cleanup all .o,  .a and executables.
#
# ******************************************************************************
clean:
	@(cd $(DIR_SRC) && $(MAKE) clean) || exit 1
	@if [ $(PACKAGE) != "ijg" ]; then \
		echo "$(RM) $(INSTALL_LIB_DIR)/*"; \
		for lib in $(LIBRARY_NAMES); do \
			$(RM) $(INSTALL_LIB_DIR)/$$lib; \
		done; \
	fi
	@if [ $(PACKAGE) != "commonnbis" ]; then \
		echo "$(RM) $(INSTALL_BIN_DIR)/*"; \
		for prog in $(PROGRAMS); do \
			$(RM) $(INSTALL_BIN_DIR)/$$prog; \
		done; \
	fi
#
# ******************************************************************************
#
# Target: catalog
# Target to create catalog file for all the libraries and
# applications.
#
# ******************************************************************************
catalog:
	@(cd $(DIR_SRC) && $(MAKE) catalog) || exit 1;\
#
# ******************************************************************************
#
# Target: help
# Target to print help manual.
#
# ******************************************************************************
#
help:
	@echo "WARNING: You must build the complete NBIS software "
	@echo "         as least one, in order to build individual "
	@echo "         package."
	@echo "For individuals wanting to build a specific package of "
	@echo "NBIS Open Source Software, the simple instructions are:"
	@echo "1.  \`make it\`"
	@echo "2.  \`make install\`"
#
