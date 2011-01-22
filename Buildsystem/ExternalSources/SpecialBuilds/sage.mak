# Matthew Henderson, 21.7.2007 (Swansea)
# Copyright 2007, 2008, 2009, 2011 Oliver Kullmann
# This file is part of the OKlibrary. OKlibrary is free software; you can redistribute 
# it and/or modify it under the terms of the GNU General Public License as published by
# the Free Software Foundation and included in this library; either version 3 of the 
# License, or any later version.

# ##################################
# Directory Structure
# ################################## 


sage_directories_okl := $(sage_base_installation_dir_okl) $(sage_datadir_okl)

.PHONY : sage cleanallsage

$(sage_directories_okl) : % : 
	mkdir -p $@

# #################################
# Main sage targets
# #################################

sage : $(sage_directories_okl)
	$(call unarchive_uncompressed,$(sage_source_okl),$(sage_base_installation_dir_okl))
	cd $(sage_base_installation_dir_okl); $(postcondition) \
	mv $(sage_recommended_okl) $(sage_recommended_version_number_okl); $(postcondition)
	cd $(sage_installation_dir_okl); $(postcondition) \
	DOT_SAGE=$(sage_datadir_okl) make; $(postcondition) \
	make test; $(postcondition) \
	ln -s --force $(sage_call_okl) $(public_bin_dir_okl)/sage


# #################################
# Cleaning
# #################################

cleanallsage : 
	-rm -rf $(sage_base_installation_dir_okl)
