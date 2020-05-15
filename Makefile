PROJECT = cpp_helper

SRCDIR = src
LIBDIR = lib
LIBTEMP = lib_temp

OUTPUT = output
OUTPUT_OBJ = outputobj

CXX = g++
AR = ar

CPPFLAGS = -g -O2 -fPIC -finline-functions -std=c++11 \
		-Wall -W -Wshadow -Wpointer-arith -Wcast-qual \
        -Wwrite-strings -Woverloaded-virtual \
		-Werror -Wno-unused-parameter -Wno-unused-function

LDFLAGS = -static

SOFILE  = lib$(PROJECT).so
AFILE = lib$(PROJECT).a
LDADDS = -lwslb

OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OUTPUT_OBJ)/%.o, $(wildcard *.cpp))
OBJ += $(patsubst $(SRCDIR)/%.c, $(OUTPUT_OBJ)/%.o, $(wildcard *.c))

DIST = $(wildcard $(SRCDIR)/*.h)
DIST += $(wildcard $(SRCDIR)/*.hh)
LIB_NAME = $(shell dir $(LIBDIR))

INCLUDE = $(foreach name, $(LIB_NAME), -I $(LIBTEMP)/$(name)/Dist)
LIB = $(foreach name, $(LIB_NAME), -L $(LIBTEMP)/$(name)/Dist)

.PHONY: all pre-install post-install clean

all : pre-install $(AFILE) post-install

$(SOFILE) : $(OBJ)
	$(CXX) $(CPPFLAGS) -Wl,-soname,$@ -o $@ $^ $(INCLUDE) $(LIB) $(LDFLAGS)

$(AFILE) : $(OBJ)
	$(AR) rcs $@ $^

$(OUTPUT_OBJ)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE) $(LIB) $(LDADDS)
	
pre-install :
	@mkdir -p ${OUTPUT} ${OUTPUT_OBJ}
	@set LANG=C
	@cp -r $(LIBDIR) $(LIBTEMP)
	$(foreach name, $(LIB_NAME), shell cd $(LIBTEMP)/$(name) && make)

post-install :
	@cp $(DIST) ${OUTPUT}
	@mv $(AFILE) $(OUTPUT)
	
clean:
	-rm -rf $(OUTPUT_OBJ)
	-rm -rf $(OUTPUT)
