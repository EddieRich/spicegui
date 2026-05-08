#==============================================================================
#
# Makefile starter
#
# Automatic Variables
#   $@ The file name of the target of the rule
#   $< The name of the first prerequisite
#   $? The names of all the prerequisites that are newer than the target
#   $^ The names of all the prerequisites
#
#==============================================================================
#
# Based on
# https://www.reddit.com/r/C_Programming/comments/1ghy847/the_perfect_makefile/
#
# also see ...
# https://www.gnu.org/software/make/manual/html_node/index.html
#
# modified flags, added .PHONY
# add OS dependant commands
#

#$(CC) defaults to cc, which defaults to gcc for *.c and g++ for *.cpp

# CFLAGS is not defined by default
CFLAGS = -MMD -Wall -Wextra -Werror -std=c17 -march=x86-64 -fdiagnostics-color=always

# linux does not define the OS environment variable
# if in the future it does, it won't be Windows_NT
ifeq ($(OS),Windows_NT)
	SRC=windows_platform.c
	EXE=spicegui.exe
	LIBS=$(addprefix -l, gdi32)
# 	RM=del
else
	SRC=linux_platform.c
	EXE=spicegui
	LIBS=$(addprefix -l,)
#RM is predefined as RM = rm -f
endif

SRC+= viewport.c
OBJ = $(SRC:%.c=%.o)
DEP = $(OBJ:%.o=%.d)

.PHONY: clean #install

debug: CFLAGS += -g
debug: $(EXE)

remake: clean debug
.NOTPARALLEL: remake

release: CFLAGS += -Os -s -fno-ident -fno-asynchronous-unwind-tables -faggressive-loop-optimizations
release: clean $(EXE)
.NOTPARALLEL: release

clean:
	rm $(OBJ) $(DEP) $(EXE)

#install: release
#	cp $(EXE) $(TARGET)/bin

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

-include $(DEP)

# the default .o:.c implicit rule is ...
#	$(CC) $(CPPFLAGS) $(CFLAGS) -c
# the -c flag generates a .o file.
#
# only define this if there are extra steps or drastic changes
# in most cases everything is handled by the flags
#
#%.o: %.c
#	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
