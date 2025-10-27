PLUGIN_NAME = sudo_io_plugin
SRC = $(PLUGIN_NAME).cpp
OUT = $(PLUGIN_NAME).so

CXX = g++
CXXFLAGS = -Wall -fPIC
LDFLAGS = -shared

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)
