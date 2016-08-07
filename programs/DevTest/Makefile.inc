
$(BINDIR)/DevTest: $$(GRAPHLIB) say-DevTest $$(BUILDDIR)/DevTest/main.o $$(patsubst $$(PROGDIR)/DevTest/%.cpp,$$(BUILDDIR)/DevTest/%.o,$(wildcard $(PROGDIR)/DevTest/*.cpp))
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/DevTest/%.o: $$(PROGDIR)/DevTest/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

