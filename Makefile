# Makefile to build multiple independent C demos with GTK4/Libadwaita

TARGET := your_app

CC      := gcc
CFLAGS  := -Wall -g $(shell pkg-config --cflags gtk4 libadwaita-1)
LDFLAGS := -lm $(shell pkg-config --libs gtk4 libadwaita-1)

SRC := main.c							\
			 your_app.c 				\
			 main_window.c 			\
			 dashboard_page.c		\
			 preferences_page.c	\
			 page_signals.c			\
			 gauge_widget.c

BUILDDIR := build
DEPDIR   := $(BUILDDIR)/.deps

# Generated resource source lives in build/
RES_XML := your_app.gresource.xml
RES_C   := $(BUILDDIR)/your_app_resources.c

OBJ := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRC)) $(BUILDDIR)/your_app_resources.o

.PHONY: all clean run

# Default target
all: $(BUILDDIR)/$(TARGET)

# Link final binary inside build/
$(BUILDDIR)/$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compilation rule: put .o and .d files in build/
$(BUILDDIR)/%.o: %.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	mv $(BUILDDIR)/$*.d $(DEPDIR)/$*.d

# Resource generation: put generated .c file in build/
RESOURCE_DEPS := $(shell glib-compile-resources --generate-dependencies $(RES_XML))

$(RES_C): $(RES_XML) $(RESOURCE_DEPS) | $(BUILDDIR)
	glib-compile-resources --generate-source --target=$@ $<

# Compile the generated resource .c into .o
$(BUILDDIR)/your_app_resources.o: $(RES_C) | $(DEPDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	mv $(BUILDDIR)/your_app_resources.d $(DEPDIR)/your_app_resources.d

# Ensure build directories exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(DEPDIR):
	mkdir -p $(DEPDIR)

# Clean up everything
clean:
	rm -rf $(BUILDDIR)

# Include dependency files
-include $(patsubst %.o,$(DEPDIR)/%.d,$(OBJ))

