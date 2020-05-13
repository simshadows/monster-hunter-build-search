CXX=g++
CXXFLAGS=-Wall -Werror -Wextra -fsanitize=address

MAINOBJECTS=src/mhwi_build_search.o
OBJECTS=src/core/build_components.o \
		src/core/sharpness_gauge.o \
		src/core/weapon_augments.o \
		src/core/weapon_upgrades.o \
		src/database/database.o \
		src/database/database_skills.o \
		src/database/database_decorations.o \
		src/database/database_weapons.o \
		src/database/database_armour.o \
		src/database/database_charms.o \
		src/support/containers.o \
		src/support/skill_contributions.o \
		src/support/build_calculations.o

TESTOBJECTS=tests/run_tests.o

.PHONY : all
all : mhwibs test

.PHONY : test
test : mhwibs-test
	./mhwibs-test

.PHONY : clean
clean :
	rm -f $(EXEC)
	rm -f $(OBJECTS)
	rm -f $(TESTOBJECTS)

mhwibs : $(OBJECTS) $(MAINOBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o mhwibs $(OBJECTS) $(MAINOBJECTS)

mhwibs-test : $(OBJECTS) $(TESTOBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o mhwibs-test $(OBJECTS) $(TESTOBJECTS)

