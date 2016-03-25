CXX=g++
CXXX=mpiCC
FLAGS= -Wall -g

BINARIES= matrix_gen matrix_stupid matrix_mpi

all: $(BINARIES)

matrix_gen: matrix_gen.cpp
	$(CXX) $(FLAGS) -o $@ $^

matrix_stupid: matrix_stupid.cpp
	$(CXX) $(FLAGS) -o $@ $^

matrix_mpi: matrix_mpi.cpp
	$(CXXX) $(FLAGS) -o $@ $^

clean:
	rm -f $(BINARIES)

