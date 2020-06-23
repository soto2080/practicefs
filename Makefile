COMPILER = gcc
FILESYSTEM_FILES = practicefs.c

build: $(FILESYSTEM_FILES)
	$(COMPILER) -D_GNU_SOURCE $(FILESYSTEM_FILES) -o practicefs `pkg-config fuse3 --cflags --libs`
	echo 'To Mount: ./practicefs -f [mount point]'

clean:
	rm practicefs