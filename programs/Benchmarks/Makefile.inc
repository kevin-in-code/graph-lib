
$(BINDIR)/Benchmarks: $$(GRAPHLIB) say-Benchmarks $$(BUILDDIR)/Benchmarks/main.o $$(patsubst $$(PROGDIR)/Benchmarks/%.cpp,$$(BUILDDIR)/Benchmarks/%.o,$(wildcard $(PROGDIR)/Benchmarks/*.cpp))
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/Benchmarks/%.o: $$(GRAPHLIB) $$(PROGDIR)/Benchmarks/%.cpp $(wildcard $(PROGDIR)/Benchmarks/*.hpp) $(wildcard $(PROGDIR)/Benchmarks/*.h)
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

