CFLAGS=-g

all: count.image iloop.image differences.image test
clean:
	$(RM) count.image iloop.image differences.image method-of-differences cavosim

%.image: %.s cavoasm.py
	./cavoasm.py < $< > $@

test: method-of-differences cavosim differences.image
	-./method-of-differences | head -11 # it outputs the initial a5
	-./cavosim differences.image | grep 'mem\[8]' | head # it doesn't
