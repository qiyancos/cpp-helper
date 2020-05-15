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

OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OUTPUT_OBJ)/%.o, \
		$(wildcard $(SRCDIR)/*.cpp))
OBJ += $(patsubst $(SRCDIR)/%.c, $(OUTPUT_OBJ)/%.o, \
	    $(wildcard $(SRCDIR)/*.c))

DIST = $(wildcard $(SRCDIR)/*.h)
DIST += $(wildcard $(SRCDIR)/*.hh)

LIB_NAME = $(patsubst $(LIBDIR)%, $(LIBTEMP)%, $(wildcard $(LIBDIR)/*))
INCLUDE = $(foreach name, $(LIB_NAME), -I$(name)/Dist)
LIB = $(foreach name, $(LIB_NAME), -L$(name)/Dist)

MAKE_PID := $(shell echo $$PPID)
JOB_FLAG := $(filter -j%, $(subst -j ,-j, \
	    $(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
JOBS     := $(subst -j,,$(JOB_FLAG))

#########################################################

.PHONY: all pre-install post-install clean

all : post-install

$(SOFILE) : $(OBJ)
	$(CXX) $(CPPFLAGS) -Wl,-soname,$@ -o $@ $^ $(INCLUDE) $(LIB) $(LDFLAGS)

$(AFILE) : $(OBJ)
	$(AR) rcs $@ $^

$(OUTPUT_OBJ)/%.o : $(SRCDIR)/%.cpp pre-install
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE) $(LIB) $(LDADDS)
	
pre-install :
	mkdir -p ${OUTPUT} ${OUTPUT_OBJ}
	set LANG=C
	@if [ x$(LIBTEMP) != x$(wildcard $(LIBTEMP)) ]; \
	then \
	    cp -r $(LIBDIR) $(LIBTEMP); \
		$(foreach name, $(LIB_NAME), cd $(name) && make $(JOB_FLAG)); \
	fi

post-install : $(AFILE)
	cp $(DIST) ${OUTPUT}
	mv $(AFILE) $(OUTPUT)
	
clean:
	rm -rf $(OUTPUT_OBJ)
	rm -rf $(OUTPUT)
	rm -rf $(LIBTEMP)
