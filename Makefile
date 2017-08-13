SRC := ./src
BIN := ./bin
CXXFILES := $(wildcard $(SRC)/*.cpp)
CXXOPTIONS := -g
ALL_CXXOPTIONS := -o $(BIN)/vsl -I$(SRC) -std=c++14 -Wall -Wextra $(CXXOPTIONS)

all: $(CXXFILES) | $(BIN)
	$(CXX) $(ALL_CXXOPTIONS) $(CXXFILES)

$(BIN):
	mkdir $(BIN)

clean:
	rm -r $(BIN)
