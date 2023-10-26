# make              # to compile files and create the executables
# make clean        # to cleanup object files and executables
# make cleanobj     # to cleanup object files only

CFLAGS = -Wall -O2 -g

PROGS = imageTool imageTest

RESOURCES = ./test

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9

all: $(PROGS)

imageTest: imageTest.o image8bit.o instrumentation.o

imageTest.o: image8bit.h instrumentation.h

imageTool: imageTool.o image8bit.o instrumentation.o

imageTool.o: image8bit.h instrumentation.h

# Rule to make any .o file dependent upon corresponding .h file
%.o: %.h

.PHONY: setup
setup: $(RESOURCES)/

$(RESOURCES)/:
	mkdir -p $@
	curl -s -o test/aed-trab1-test.zip https://sweet.ua.pt/mario.antunes/aed/test/aed-trab1-test.zip
	unzip -q -o test/aed-trab1-test.zip -d test/

test1: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm neg save neg.pgm
	cmp neg.pgm $(RESOURCES)/neg.pgm

test2: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm thr 128 save thr.pgm
	cmp thr.pgm $(RESOURCES)/thr.pgm

test3: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm bri .33 save bri.pgm
	cmp bri.pgm $(RESOURCES)/bri.pgm

test4: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm rotate save rotate.pgm
	cmp rotate.pgm $(RESOURCES)/rotate.pgm

test5: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm mirror save mirror.pgm
	cmp mirror.pgm $(RESOURCES)/mirror.pgm

test6: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm crop 100,100,100,100 save crop.pgm
	cmp crop.pgm $(RESOURCES)/crop.pgm

test7: $(PROGS) setup
	./imageTool $(RESOURCES)/small.pgm $(RESOURCES)/original.pgm paste 100,100 save paste.pgm
	cmp paste.pgm $(RESOURCES)/paste.pgm

test8: $(PROGS) setup
	./imageTool $(RESOURCES)/small.pgm $(RESOURCES)/original.pgm blend 100,100,.33 save blend.pgm
	cmp blend.pgm $(RESOURCES)/blend.pgm

test9: $(PROGS) setup
	./imageTool $(RESOURCES)/original.pgm blur 7,7 save blur.pgm
	cmp blur.pgm $(RESOURCES)/blur.pgm

test: $(PROGS) $(TESTS)

# Make uses builtin rule to create .o from .c files.

cleanobj:
	rm -f *.o

clean: cleanobj
	rm -f $(PROGS)
	rm -Rf $(RESOURCES)

