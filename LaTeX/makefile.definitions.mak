# Tony H. Bao, 13.04.2005



ifdef OKMESSAGES
  OKMessages := -I$(OKMESSAGES)
else
  OKMessages :=
endif

ifdef OKIOTOOLS
  OKIOtools := -I$(OKIOTOOLS)
else
  OKIOtools :=
endif

ifdef OKSYSTEM
  OKSystem := -I$(OKSYSTEM)
else
  OKSystem := 
endif

OKTestsystem := -I$(OKSYSTEM)/Transitional/Testsystem

General_options := -g
Optimisation_options := -O3 -DNDEBUG

test_program := TestLaTeX

programs := 

source_libraries = $(Boost) $(OKTestsystem) $(OKSystem)

link_libraries := 

Root := $(wildcard ../../..)
