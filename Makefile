SRCDIR = src
STRUCTURE = $(shell cd $(SRCDIR) && find . -type d)

DEPSDIR = thirdparty

CXX ?= g++
CXXFLAGS ?= -g $(INCLUDES)

BINARYDIR = bin
OBJECTDIR = $(BINARYDIR)/obj

TARGET = $(BINARYDIR)/rgb_lights

LIB_DIR += 

LIBS += -lpthread
LIBS += -latomic
LIBS += -lsioclient
LIBS += -lssl
LIBS += -lcrypto
LIBS += -lhomecontroller
LIBS += -lpigpio

# drivers
_HEADERS += drivers/driver.h

_OBJECTS += drivers/pwm_driver.o
_HEADERS += drivers/pwm_driver.h

_OBJECTS += drivers/test_driver.o
_HEADERS += drivers/test_driver.h

# programs
_OBJECTS += programs/default_program.o
_HEADERS += programs/default_program.h

_OBJECTS += programs/program.o
_HEADERS += programs/program.h

_OBJECTS += programs/rainbow_fade_program.o
_HEADERS += programs/rainbow_fade_program.h

# root
_OBJECTS += config.o
_HEADERS += config.h

_OBJECTS += main.o

_OBJECTS += pwm.o
_HEADERS += pwm.h

_OBJECTS += rgb_lights.o
_HEADERS += rgb_lights.h

OBJECTS = $(patsubst %,$(OBJECTDIR)/%,$(_OBJECTS))
HEADERS = $(patsubst %,$(SRCDIR)/%,$(_HEADERS))

$(OBJECTDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJECTDIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJECTDIR):
	mkdir -p $(OBJECTDIR)
	mkdir -p $(addprefix $(OBJECTDIR)/,$(STRUCTURE))

clean:
	rm -rf bin

.PHONY: clean