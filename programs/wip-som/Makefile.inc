
$(BINDIR)/wip-som: $$(GRAPHLIB) say-wip-som $$(BUILDDIR)/wip-som/main.o $$(patsubst $$(PROGDIR)/wip-som/%.cpp,$$(BUILDDIR)/wip-som/%.o,$(wildcard $(PROGDIR)/wip-som/*.cpp))
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/wip-som/%.o: $$(PROGDIR)/wip-som/%.cpp $(wildcard $(PROGDIR)/wip-som/*.hpp) $(wildcard $(PROGDIR)/wip-som/*.h)
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

