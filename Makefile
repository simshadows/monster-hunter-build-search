CXXFLAGSBASE=-Wall -Werror -Wextra -std=c++17

#CXX=g++
CXXFLAGS=$(CXXFLAGSBASE) -O3 -DNDEBUG

EXEC=mhwibs
MAINOBJECTS=src/mhwi_build_search.o
OBJECTS=src/search.o \
		src/search_jsonparse.o \
		src/core/src/build_components.o \
		src/core/src/sharpness_gauge.o \
		src/core/src/weapon_augments.o \
		src/core/src/weapon_upgrades.o \
		src/support/src/containers_armour_equips.o \
		src/support/src/containers_deco_equips.o \
		src/support/src/containers_skill_map.o \
		src/support/src/containers_skill_spec.o \
		src/support/src/containers_weapon_instance.o \
		src/support/src/skill_contributions.o \
		src/support/src/build_calculations.o \
		src/utils/src/logging.o \
		src/database/src/database.o \
		src/database/src/database_skills.o \
		src/database/src/database_decorations.o \
		src/database/src/database_weapons.o \
		src/database/src/database_armour.o \
		src/database/src/database_charms.o

TESTEXEC=mhwibs-test
TESTOBJECTS=tests/run_tests.o

.PHONY : all
all : $(EXEC) test

.PHONY : asserts
asserts : CXXFLAGS=$(CXXFLAGSBASE) -O3
asserts : all

.PHONY : debug
debug : CXXFLAGS=$(CXXFLAGSBASE) -fsanitize=address -g -Og
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

.PHONY : asserts2
asserts2 : CXX=clang++-9
asserts2 : asserts

.PHONY : debug2
debug2 : CXX=clang++-9
debug2 : debug

