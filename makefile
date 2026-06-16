CC=clang
CFLAGS=-MMD -Wall -Wextra -Werror -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-format-overflow -std=c17 -march=x86-64 -fdiagnostics-color=always
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)
DEP=$(OBJ:%.o=%.d)

EXE=spicegui

# add any additional libraries here ...
# $(addprefix -l, m pthread GL)
LIBS=$(addprefix -l, X11 m)

.PHONY: clean

debug: CFLAGS += -O0 -g -gdwarf
debug: $(EXE)

remake: clean debug
.NOTPARALLEL: remake

release: CFLAGS += -O2
release: clean $(EXE)
.NOTPARALLEL: release

clean:
	$(RM) $(OBJ) $(DEP) $(EXE)

install: release
	sudo cp $(EXE) /usr/local/bin/$(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

-include $(DEP)
