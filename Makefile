
all: example
.PHONY: example clean

example:
	$(MAKE) -C example

clean:
	$(MAKE) -C example clean
