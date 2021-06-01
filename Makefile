# Copyright (C) 2021 Ryan Britton
#
# This file is part of Goodnight Mouse.
#
# Goodnight Mouse is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Goodnight Mouse is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.

TARGET_DIR := ./bin
BUILD_DIR := ./build
SRC_DIR := ./src

TARGET_EXEC := $(TARGET_DIR)/goodnight_mouse

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

CC := gcc
CFLAGS := -Wall

CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)
CFLAGS += $(shell pkg-config --cflags atspi-2)
LDFLAGS += $(shell pkg-config --libs atspi-2)
CFLAGS += $(shell pkg-config --cflags gobject-2.0)
LDFLAGS += $(shell pkg-config --libs gobject-2.0)
CFLAGS += $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS += $(shell pkg-config --libs gtk+-3.0)

build: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_EXEC) $^

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)