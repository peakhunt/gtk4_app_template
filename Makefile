# Makefile to build multiple independent C demos with GTK4/Libadwaita

# List all your demo source files here
TARGET := your_app

CC      := gcc
CFLAGS  := -Wall -g $(shell pkg-config --cflags gtk4 libadwaita-1)
LDFLAGS := $(shell pkg-config --libs gtk4 libadwaita-1)

SRC := main.c your_app.c main_window.c your_app_resources.c

DEPDIR := .deps

OBJ := $(SRC:.c=.o)

.PHONY: all clean run

# Default target: build all demos
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compilation
%.o: %.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	mv $*.d $(DEPDIR)/$*.d


RESOURCE_DEPS := $(shell glib-compile-resources --generate-dependencies your_app.gresource.xml)

your_app_resources.c: your_app.gresource.xml $(RESOURCE_DEPS)
	glib-compile-resources --generate-source --target=$@ $<

clean:
	rm -f $(TARGET) $(OBJ) your_app_resources.c
	rm -rf $(DEPDIR)

$(DEPDIR):
	mkdir -p $(DEPDIR)

-include $(patsubst %.o,$(DEPDIR)/%.d,$(OBJ))
