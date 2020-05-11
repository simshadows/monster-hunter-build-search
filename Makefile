CXX=g++
CXXFLAGS=-Wall -Werror -Wextra -fsanitize=address

EXEC=mhwibs
SRCDIR=src
OBJECTS=$(SRCDIR)/mhwi_build_search.o \
		$(SRCDIR)/core/build_components.o \
		$(SRCDIR)/core/sharpness_gauge.o \
		$(SRCDIR)/core/weapon_augments.o \
		$(SRCDIR)/database/database.o \
		$(SRCDIR)/database/database_skills.o \
		$(SRCDIR)/database/database_decorations.o \
		$(SRCDIR)/database/database_weapons.o \
		$(SRCDIR)/database/database_armour.o \
		$(SRCDIR)/database/database_charms.o \
		$(SRCDIR)/support/containers.o \
		$(SRCDIR)/support/skill_contributions.o

.PHONY : all
all :
	$(MAKE) $(EXEC)

.PHONY : clean
clean :
	rm -f $(EXEC)
	rm -f $(OBJECTS)

$(EXEC) : $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(EXEC) $(OBJECTS)

