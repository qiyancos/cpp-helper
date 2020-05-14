CXX = g++
AR = ar

CPPFLAGS = -g -O2 -fPIC -finline-functions -std=c++11\
		-Wall -W -Wshadow -Wpointer-arith -Wcast-qual \
        -Wwrite-strings -Woverloaded-virtual \
		-Werror -Wno-unused-parameter -Wno-unused-function \
	    -I ./lib/FreeImagePlus

LDFLAGS = -static

INCLUDE =
LIB =
DIST = md5_helper.h conf_helper.h image_helper.h

DPOINT = 0
ifeq ($(DPOINT),1)
	CPPFLAGS += -D_USE_DOUBLE_POINT_
	SOFILE  = libprogram_util.so
	AFILE = libprogram_util.a
	LDADDS = -lwslb_d
else
	SOFILE  = libprogram_util.so
	AFILE = libprogram_util.a
	LDADDS = -lwslb
endif

OUTPUT = output
OUTPUT_OBJ = outputobj

OBJ = $(patsubst %.cpp, $(OUTPUT_OBJ)/%.o,$(wildcard *.cpp))
OBJ += $(patsubst %.c, $(OUTPUT_OBJ)/%.o,$(wildcard *.c))

.PHONY: all pre-install post-install clean

all : pre-install $(AFILE) post-install

$(SOFILE) : $(OBJ)
	$(CXX) $(CPPFLAGS) -Wl,-soname,$@ -o $@ $^ $(INCLUDE) $(LIB) $(LDFLAGS)

$(AFILE) : $(OBJ)
	$(AR) rcs $@ $^

$(OUTPUT_OBJ)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE) $(LIB) $(LDADDS)
	
pre-install :
	@mkdir -p ${OUTPUT} ${OUTPUT_OBJ}; set LANG=C
	
post-install :
	cp $(DIST) ${OUTPUT}
	mv $(AFILE) $(OUTPUT)
	
clean:
	-rm -rf $(OUTPUT_OBJ)
	-rm -rf $(OUTPUT)
