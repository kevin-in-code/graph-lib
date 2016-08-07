
$(BINDIR)/Main_TCS_2015: $$(GRAPHLIB) say-Main_TCS_2015 $$(BUILDDIR)/Main_TCS_2015/main.o $$(patsubst $$(PROGDIR)/Main_TCS_2015/%.cpp,$$(BUILDDIR)/Main_TCS_2015/%.o,$(wildcard $(PROGDIR)/Main_TCS_2015/*.cpp))
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^) $(filter %.a,$^)

$(BUILDDIR)/Main_TCS_2015/%.o: $$(PROGDIR)/Main_TCS_2015/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

