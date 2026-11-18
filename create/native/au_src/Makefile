CC=gcc
CFLAGS=-O3 -march=native -Wall -I../include
SHAKE_OBJ=../shake256.o

GEN_OBJS=keygen.o mint.o serialgen.o gentdrow.o
MAIN_OBJS=binaryproof.o hash.o ringsign.o spend.o verify.o audit.o
MISC_OBJS=bignum.o gnorm.o
POLY_OBJS=poly_mult.o poly_q.o poly_qbig.o
RNG_OBJS=fastrandombytes.o randombytes.o
SAMPLER_OBJS=sammat.o sampleb.o samplex.o

OBJS=$(GEN_OBJS) $(MAIN_OBJS) $(MISC_OBJS) $(POLY_OBJS) $(RNG_OBJS) $(SAMPLER_OBJS) cpucycles.o test.o

PROGRAM=ringct

all: $(PROGRAM)

$(PROGRAM): $(OBJS) $(SHAKE_OBJ)
	$(CC) $(OBJS) $(SHAKE_OBJ) -o $(PROGRAM)

$(SHAKE_OBJ): ../shake256.c
	$(CC) $(CFLAGS) -c ../shake256.c -o $@

clean:;
	rm -f *.o $(SHAKE_OBJ) $(PROGRAM)
