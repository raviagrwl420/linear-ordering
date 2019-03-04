# Compiler
CC := g++



## CPLEX

# CPLEX Installation Path
CPLEX_INSTALLATION_PATH := /opt/ibm/ILOG/CPLEX_Studio128

# CPLEX Directories
CPLEXDIR := $(CPLEX_INSTALLATION_PATH)/cplex
CONCERTDIR := $(CPLEX_INSTALLATION_PATH)/concert

# CPLEX Include Directories
CPLEXINCDIR := $(CPLEXDIR)/include
CONCERTINCDIR := $(CONCERTDIR)/include

# CPLEX Library
SYSTEM := x86-64_linux
LIBFORMAT := static_pic

CPLEXLIBDIR := $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR := $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)



## Project

# Source Extention
CPP_SRCEXT := cpp
C_SRCEXT := c

# Project Directories
IDIR := include
SRCDIR := src
BUILDDIR := build

# Sources
CPP_SOURCES := $(shell find $(SRCDIR) -type f -name *.$(CPP_SRCEXT))
C_SOURCES := $(shell find $(SRCDIR) -type f -name *.$(C_SRCEXT))

# Objects
CPP_OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(CPP_SOURCES:.$(CPP_SRCEXT)=.o))
C_OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(C_SOURCES:.$(C_SRCEXT)=.o))
OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS)

# Target
TARGET := built



## Flags

# Include Directories
INCDIRS := -I $(IDIR) -I $(CPLEXINCDIR) -I $(CONCERTINCDIR)

# Linker Directories
LNDIRS := -L $(CPLEXLIBDIR) -L $(CONCERTLIBDIR)

# Linker Flags
LNFLAGS := -lilocplex -lconcert -lcplex -lm -lpthread -ldl

# CPP Flags
CPPFLAGS := -g -std=c++11 -w -DIL_STD $(INCDIRS) $(LNDIRS) $(LNFLAGS)



## Build Statements

# Build Executable
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(CPPFLAGS)

# Build Object Files
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(CPP_SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

# Build C Object Files
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(C_SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

# Clean
clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
