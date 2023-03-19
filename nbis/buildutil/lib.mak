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
# Filename:             lib.mak
# Integrators:          Kenneth Ko
# Organization:         NIST/ITL
# Host System:          GNU GCC/GMAKE GENERIC (UNIX)
# Date Created:         08/20/2006
#
# ******************************************************************************
#
# Generic Makefile to build library.
#
# ******************************************************************************
OBJDIR := $(subst $(DIR_SRC_LIB),$(DIR_OBJ_SRC_LIB),$(CURDIR))
#
OBJFILES := $(SRC:%.c=$(OBJDIR)/%.o)
#
DEPFILES := $(SRC:%.c=$(OBJDIR)/%.d)
#
ARFLAGS := ru
AR      := ar $(ARFLAGS)
#
it: $(OBJFILES) $(INSTALL_LIB_DIR)/$(LIBRARY).a
#
$(OBJDIR)/%.o: %.c
	$(call make-depend,$<,$@,$(subst .o,.d,$@),$(notdir $@))
	$(CCC) -I$(DIR_INC) $(EXT_INCS) -c -o $@ $<
#
define make-depend
	@$(CC) $(CFLAGS) -I$(DIR_INC) $(EXT_INCS) $(M) $1 | \
	$(SED) 's,^$4 *:, $2 $3:,' > $3.tmp
	@$(SED) -e 's/#.*//' \
		-e 's/^[^:]*: *//' \
		-e 's/ *\\$$//' \
		-e '/^$$/ d' \
		-e 's/$$/ :/' \
		-e 's/ : :/ :/' $3.tmp >> $3.tmp
	@$(MV) $3.tmp $3
endef
#
$(INSTALL_LIB_DIR)/$(LIBRARY).a: $(OBJFILES)
	$(AR) $@ $?
	$(CP) $(INSTALL_LIB_DIR)/$(LIBRARY).a $(EXPORTS_LIB_DIR)
#
config:
	@for depfile in $(DEPFILES); do \
		echo "Creating \"$$depfile\"...."; \
		$(TOUCH) $$depfile; \
	done
#
install:
	$(INSTALL_LIB) $(INSTALL_LIB_DIR)/$(LIBRARY).a $(INSTALL_ROOT_LIB_DIR)
#
clean:
	@echo "$(RM) $(OBJDIR)/*.o"
	@for objfile in $(OBJFILES); do \
		$(RM) $$objfile; \
	done; \
	$(RM) $(INSTALL_LIB_DIR)/*.a
	$(RM) $(DIR_SRC_LIB)/*/*.txt
#
catalog:
	@/bin/sh $(DIR_ROOT_BUILDUTIL)/catalog.sh \
		proc $(DIR_SRC_LIB)/$(LIBRARY) c > /dev/null 2>&1
	$(MV) catalog.txt $(LIBRARY).txt
	$(CP) $(LIBRARY).txt $(DOC_CATS_DIR)
#
-include $(DEPFILES)
#
