#
# Copyright 2014 Onat Turkcuoglu. All rights reserved.
# License: http://www.opensource.org/licenses/BSD-2-Clause
#
# Edited from bgfx

UNAME := $(shell uname)
ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin))
ifeq ($(UNAME),$(filter $(UNAME),Darwin))
OS=darwin
else
OS=linux
endif
else
OS=windows
endif

GENIE=../bx/tools/bin/$(OS)/genie

all:
	$(GENIE) vs2012
	$(GENIE) vs2013
