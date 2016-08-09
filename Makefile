SRCDIR      := src
INCDIR      := include
BUILDDIR    := build
LIBDIR      := lib
BINDIR      := bin
PROGDIR     := programs

CXX=g++
CXXFLAGS=-std=c++11 -W -Wextra -pedantic -O3 -I$(INCDIR) -L$(LIBDIR)
AR=ar

GRAPHLIB=$(LIBDIR)/libgraphlib.a
GRAPHLIBLINK=graphlib

define DEFAULT_MAKEFILE_INC
$$(BINDIR)/$T: $$$$(GRAPHLIB) say-$T $$$$(BUILDDIR)/$T/main.o $$$$(patsubst $$$$(PROGDIR)/$T/%.cpp,$$$$(BUILDDIR)/$T/%.o,$$(wildcard $$(PROGDIR)/$T/*.cpp))
	$$(CXX) $$(CXXFLAGS) -o $$@ $$(filter %.o,$$^) $$(filter %.a,$$^)

$$(BUILDDIR)/$T/%.o: $$$$(PROGDIR)/$T/%.cpp $$(wildcard $$(PROGDIR)/$T/*.hpp) $$(wildcard $$(PROGDIR)/$T/*.h)
	$$(CXX) $$(CXXFLAGS) -c $$(filter %.cpp,$$^) -o $$@
endef

MAKEFILES=$(patsubst %,$(PROGDIR)/%/Makefile.inc,$(sort $(notdir $(patsubst %/,%,$(wildcard $(PROGDIR)/*)))))
PROGNAMES=$(sort $(notdir $(patsubst %/,%,$(dir $(MAKEFILES)))))
PROGBINFILES=$(addprefix $(BINDIR)/,$(PROGNAMES))
SAYPROGNAMES=$(patsubst %,say-%,$(PROGNAMES))

CPPFILES=$(wildcard $(SRCDIR)/*.cpp)
OBJFILES=$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(CPPFILES))
HEADERS=$(wildcard $(INCDIR)/*.hpp) $(wildcard $(INCDIR)/*.h) $(wildcard $(SRCDIR)/*.hpp) $(wildcard $(SRCDIR)/*.h)

.SECONDEXPANSION:
.PHONY: clean cleanall say-graphlib $(SAYPROGNAMES)

all: $(GRAPHLIB) $(PROGBINFILES)

$(MAKEFILES): | Makefile
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo '';) ) >$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo '$$(BINDIR)/$T: $$$$(GRAPHLIB) say-$T $$$$(BUILDDIR)/$T/main.o $$$$(patsubst $$$$(PROGDIR)/$T/%.cpp,$$$$(BUILDDIR)/$T/%.o,$$(wildcard $$(PROGDIR)/$T/*.cpp))';) ) >>$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo -e '\t$$(CXX) $$(CXXFLAGS) -o $$@ $$(filter %.o,$$^) $$(filter %.a,$$^)';) ) >>$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo '';) ) >>$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo '$$(BUILDDIR)/$T/%.o: $$$$(PROGDIR)/$T/%.cpp $$(wildcard $$(PROGDIR)/$T/*.hpp) $$(wildcard $$(PROGDIR)/$T/*.h)';) ) >>$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo -e '\t$$(CXX) $$(CXXFLAGS) -c $$(filter %.cpp,$$^) -o $$@';) ) >>$@
	@( $(foreach T,$(patsubst $(PROGDIR)/%/Makefile.inc,%,$@),echo '';) ) >>$@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LIBDIR): $(BUILDDIR)
	mkdir -p $(LIBDIR)

$(BINDIR): $(BUILDDIR)
	mkdir -p $(BINDIR)

say-graphlib: $(LIBDIR)
	@echo "======================================================================"
	@echo "GraphLib"
	@echo "----------------------------------------------------------------------"

$(SAYPROGNAMES): $(BINDIR)
	@echo "======================================================================"
	@echo $(patsubst say-%,%,$@)
	@echo "----------------------------------------------------------------------"
	mkdir -p $(BUILDDIR)/$(patsubst say-%,%,$@)

$(GRAPHLIB): say-graphlib $(OBJFILES)
	$(AR) rvs $(GRAPHLIB) $(filter %.o,$^)

$(OBJFILES): $$(patsubst $$(BUILDDIR)/%.o,$$(SRCDIR)/%.cpp,$$@) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

clean:
	rm -rf $(BUILDDIR)

cleanall: clean
	rm -rf $(LIBDIR)
	rm -rf $(BINDIR)

-include $(MAKEFILES)
