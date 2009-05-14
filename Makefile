CFLAGS=-g

all: count.image iloop.image differences.image test
clean:
	$(RM) count.image iloop.image differences.image method-of-differences dumbasssim

%.image: %.s dumbassasm.py
	./dumbassasm.py < $< > $@

test: method-of-differences dumbasssim differences.image
	-./method-of-differences | head -11 # it outputs the initial a5
	-./dumbasssim differences.image | grep 'mem\[8]' | head # it doesn't
