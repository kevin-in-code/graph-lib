
$(BINDIR)/CountMaximalCliques: $$(GRAPHLIB) say-CountMaximalCliques $$(BUILDDIR)/CountMaximalCliques/main.o $$(patsubst $$(PROGDIR)/CountMaximalCliques/%.cpp,$$(BUILDDIR)/CountMaximalCliques/%.o,$(wildcard $(PROGDIR)/CountMaximalCliques/*.cpp))
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/CountMaximalCliques/%.o: $$(GRAPHLIB) $$(PROGDIR)/CountMaximalCliques/%.cpp $(wildcard $(PROGDIR)/CountMaximalCliques/*.hpp) $(wildcard $(PROGDIR)/CountMaximalCliques/*.h)
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

