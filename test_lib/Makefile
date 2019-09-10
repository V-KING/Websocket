# Edit by liubaolong@20190828

# !!!=== cross compile...
CROSS_COMPILE ?= 

CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

ARFLAGS = -cr
RM     = -rm -rf
MAKE   = make

CFLAGS  = 
LDFLAGS = 
DEFS    =
LIBS    =

# !!!===
# target executable file or .a or .so
target = main

# !!!===
# compile flags
CFLAGS += -Wall -Wfatal-errors

# !!!=== pkg-config here
#CFLAGS += $(shell pkg-config --cflags --libs glib-2.0 gattlib)
#LDFLAGS += $(shell pkg-config --cflags --libs glib-2.0 gattlib)

#****************************************************************************
# debug can be set to y to include debugging info, or n otherwise
debug  = y

#****************************************************************************

ifeq ($(debug), y)
    CFLAGS += -g -std=gnu9x -w -DRUPIFY -fPIC
else
    CFLAGS += -O2 -s
endif

# !!!===
DEFS    += -DJIMKENT 

CFLAGS  += $(DEFS)
CXXFLAGS = $(CFLAGS)

LIBS    += 

LDFLAGS += $(LIBS)


# !!!===
DIR_Websocket = ../
INC = ./ ../inc $(DIR_Websocket)

INCDIRS := $(addprefix -I, $(INC))

# !!!===
CFLAGS += $(INCDIRS)
CXXFLAGS += -std=c++11 

# !!!===
LDFLAGS += -lpthread -lrt -L$(DIR_Websocket) -L./ -lwebsocket -Wl,-rpath=$(DIR_Websocket) -Wl,-rpath=./

DYNC_FLAGS += -fpic -shared

# !!!===
# source file(s), including c file(s) or cpp file(s)
SRC_DIRS = .
SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp' -or -name '*.c')

OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRCS))) 


# !!!===
# in case all .c/.cpp need g++...
# CC = $(CXX)
ifeq ($(V),1)
Q=
NQ=true
else
Q=@
NQ=echo
endif

###############################################################################

all: $(target)

$(target): $(OBJS)

ifeq ($(suffix $(target)), .so)
	@$(NQ) "Generating dynamic lib file..." $(notdir $(target))
	$(Q)$(CC) $(CCFLAGS) $^ -o $(target) $(LDFLAGS) $(DYNC_FLAGS)
else ifeq ($(suffix $(target)), .a)
	@$(NQ) "Generating static lib file..." $(notdir $(target))
	$(Q)$(AR) $(ARFLAGS) -o $(target) $^
else
	@$(NQ) "Generating executable file..." $(notdir $(target))
	$(Q)$(CC) $(CCFLAGS) $^ -o $(target) $(LDFLAGS)
endif

# make all .c or .cpp
%.o: %.c
	@$(NQ) "Compiling: " $(addsuffix .c, $(basename $(notdir $@)))
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@$(NQ) "Compiling: " $(addsuffix .cpp, $(basename $(notdir $@)))
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(NQ) "Cleaning..."
	$(Q)$(RM) $(target) $(OBJS)

.PHONY: all clean
