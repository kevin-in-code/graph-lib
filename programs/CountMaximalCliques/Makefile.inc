
$(BINDIR)/CountMaximalCliques: $$(GRAPHLIB) say-CountMaximalCliques $$(BUILDDIR)/CountMaximalCliques/main.o $$(patsubst $$(PROGDIR)/CountMaximalCliques/%.cpp,$$(BUILDDIR)/CountMaximalCliques/%.o,$(wildcard $(PROGDIR)/CountMaximalCliques/*.cpp))
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/CountMaximalCliques/%.o: $$(PROGDIR)/CountMaximalCliques/%.cpp $(wildcard $(PROGDIR)/CountMaximalCliques/*.hpp) $(wildcard $(PROGDIR)/CountMaximalCliques/*.h)
	$(CXX) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

