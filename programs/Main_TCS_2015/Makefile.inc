
$(BINDIR)/Main_TCS_2015: $$(GRAPHLIB) say-Main_TCS_2015 $$(BUILDDIR)/Main_TCS_2015/main.o $$(patsubst $$(PROGDIR)/Main_TCS_2015/%.cpp,$$(BUILDDIR)/Main_TCS_2015/%.o,$(wildcard $(PROGDIR)/Main_TCS_2015/*.cpp))
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/Main_TCS_2015/%.o: $$(PROGDIR)/Main_TCS_2015/%.cpp $(wildcard $(PROGDIR)/Main_TCS_2015/*.hpp) $(wildcard $(PROGDIR)/Main_TCS_2015/*.h)
	$(CXX) $(CXXBUILD) $(CXXFLAGS) -c $(filter %.cpp,$^) -o $@

