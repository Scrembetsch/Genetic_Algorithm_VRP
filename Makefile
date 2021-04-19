SRC_DIR := Genetic_Algorithm_VRP/src
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,%.o,$(SRC_FILES))
LDFLAGS := -lm -fopenmp
CXXFLAGS := -Wall -fopenmp -Wextra -Werror -pedantic -g

.DEFAULT_GOAL := VRP

test: VRP
	./VRP

VRP: $(OBJ_FILES)
	g++ $(LDFLAGS) -o $@ $^

%.o: $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	-rm *.o VRP