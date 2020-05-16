#CXX=g++
CXXFLAGS=-Wall -Werror -Wextra -O3 -DNDEBUG

EXEC=mhwibs
MAINOBJECTS=src/mhwi_build_search.o
OBJECTS=src/search.o \
		src/search_jsonparse.o \
		src/support/containers.o \
		src/support/skill_contributions.o \
		src/support/build_calculations.o \
		src/core/build_components.o \
		src/core/sharpness_gauge.o \
		src/core/weapon_augments.o \
		src/core/weapon_upgrades.o \
		src/database/database.o \
		src/database/database_skills.o \
		src/database/database_decorations.o \
		src/database/database_weapons.o \
		src/database/database_armour.o \
		src/database/database_charms.o \
		src/utils/logging.o

TESTEXEC=mhwibs-test
TESTOBJECTS=tests/run_tests.o

.PHONY : all
all : $(EXEC) test

.PHONY : debug
debug : CXXFLAGS=-Wall -Werror -Wextra -fsanitize=address -O3
debug : all

.PHONY : test
test : $(TESTEXEC)
	./mhwibs-test

.PHONY : clean
clean :
	rm -f $(EXEC)
	rm -f $(TESTEXEC)
	rm -f $(OBJECTS)
	rm -f $(MAINOBJECTS)
	rm -f $(TESTOBJECTS)

$(EXEC) : $(OBJECTS) $(MAINOBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(EXEC) $(OBJECTS) $(MAINOBJECTS)

$(TESTEXEC) : $(OBJECTS) $(TESTOBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(TESTEXEC) $(OBJECTS) $(TESTOBJECTS)

##################################################################################
# I use this specific compiler a lot on my local machine for additional testing. #
##################################################################################

.PHONY : all2
all2 : CXX=clang++-9
all2 : all

.PHONY : debug2
debug2 : CXX=clang++-9
debug2 : debug

