objdir = .obj

cc-source = $(wildcard src/*.cpp)
test-source = $(wildcard test/*.cpp)

CXXFLAGS += -Iinclude -std=c++23

ifeq ($(config), test)
CXXFLAGS += -g -DDEBUG
source = $(filter-out src/main.cpp,$(cc-source))
source += $(test-source)
else
source = $(cc-source)
ifeq ($(config), debug)
CXXFLAGS += -g -DDEBUG
else
endif
endif

objects = $(addprefix $(objdir)/,$(patsubst %.cpp,%.o,$(source)))

cc: $(objdir) $(objects)
	@$(CXX) $(CXXFLAGS) $(objects) -o $@ -lgtest

.PHONY: clean
clean:
	rm -rf $(objdir) cc

$(objdir):
	mkdir -p $@ $@/src $@/test

includes = $(filter-out cc.o: ,$(shell g++ -MM include/cc.h))

$(objdir)/%.o: %.cpp $(includes)
	@echo $<...
	@$(CXX) $(CXXFLAGS) -c $< -o $@
