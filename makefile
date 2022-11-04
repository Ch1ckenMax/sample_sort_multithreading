EXE=psort

all: $(EXE)

psort: psort.c
	gcc $< -o $@ 