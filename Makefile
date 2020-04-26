CXX=g++
CXXFLAGS=-Wall -Werror -Wextra

EXEC=mhwibs
SRCDIR=src

OBJECTS=$(SRCDIR)/mhwi_build_search.o \
		$(SRCDIR)/utils.o

.PHONY : all
all :
	$(MAKE) $(EXEC)

$(EXEC) : $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(EXEC) $(OBJECTS)

