
$(BINDIR)/CountMaximalCliques: $$(GRAPHLIB) say-CountMaximalCliques $$(BUILDDIR)/CountMaximalCliques/main.o
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/CountMaximalCliques/main.o: $$(PROGDIR)/CountMaximalCliques/main.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
