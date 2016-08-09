
$(BINDIR)/DevTest: $$(GRAPHLIB) say-DevTest $$(BUILDDIR)/DevTest/main.o $$(patsubst $$(PROGDIR)/DevTest/%.cpp,$$(BUILDDIR)/DevTest/%.o,$(wildcard $(PROGDIR)/DevTest/*.cpp))
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/DevTest/%.o: $$(PROGDIR)/DevTest/%.cpp $(wildcard $(PROGDIR)/DevTest/*.hpp) $(wildcard $(PROGDIR)/DevTest/*.h)
	$(CXX) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

