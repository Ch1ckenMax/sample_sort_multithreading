COMPILER = gcc

FLAGS = -pthread

OBJ = psort_3035745037.o thread_info.o pivot_info.o

TARGET = psort

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(COMPILER) $(FLAGS) -o $(TARGET) $(OBJ)

seqsort : seqsort.c
	gcc seqsort.c -o seqsort