SRC := ./src
BIN := ./bin
CXXFILES := $(wildcard $(SRC)/*.cpp)
LLVMOPTIONS := $(shell llvm-config --cxxflags --ldflags --system-libs --libs \
    core)
CXXOPTIONS := -g
ALL_CXXOPTIONS := $(LLVMOPTIONS) -std=c++14 -Wall -Wextra $(CXXOPTIONS)

all: $(CXXFILES) | $(BIN)
	$(CXX) -o $(BIN)/vsl $(CXXFILES) $(ALL_CXXOPTIONS)

$(BIN):
	mkdir $(BIN)

clean:
	rm -r $(BIN)
