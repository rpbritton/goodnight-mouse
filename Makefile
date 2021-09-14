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

CFLAGS := -Wall
LDFLAGS :=
LIBS := glib-2.0 atspi-2 gobject-2.0 gtk+-3.0 gsl

#LIBS += x11 xi xkbcommon xtst
#CFLAGS += -DUSE_X11=1

LIBS += xcb
CFLAGS += -DUSE_XCB=1

CFLAGS += -DG_LOG_DOMAIN=\"GoodnightMouse\"

CFLAGS += $(shell pkg-config --cflags $(LIBS))
LDFLAGS += $(shell pkg-config --libs $(LIBS))

.DEFAULT_GOAL := build

debug: CFLAGS += -g
debug: build

build: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_EXEC) $^ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)
