CXX=g++
CXXFLAGS=-Wall -Werror -Wextra

EXEC=mhwibs
SRCDIR=src
OBJECTS=$(SRCDIR)/mhwi_build_search.o \
		$(SRCDIR)/database/database.o \
		$(SRCDIR)/database/database_skills.o \
		$(SRCDIR)/database/database_weapons.o \
		$(SRCDIR)/database/database_armour.o \
		$(SRCDIR)/containers.o \
		$(SRCDIR)/skill_contributions.o

.PHONY : all
all :
	$(MAKE) $(EXEC)

.PHONY : clean
clean :
	rm -f $(EXEC)
	rm -f $(OBJECTS)

$(EXEC) : $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(EXEC) $(OBJECTS)

