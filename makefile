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
SRCEXT := cpp

# Project Directories
IDIR := include
SRCDIR := src
BUILDDIR := build

# Sources
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# Objects
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

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
CPPFLAGS := -std=c++11 -w -DIL_STD $(INCDIRS) $(LNDIRS) $(LNFLAGS)



## Build Statements

# Build Executable
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(CPPFLAGS)

# Build Object Files
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

# Clean
clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
