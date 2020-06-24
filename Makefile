COMPILER = g++ -std=c++17
FILESYSTEM_FILES = practicefs.cpp

build: $(FILESYSTEM_FILES)
	$(COMPILER) $(FILESYSTEM_FILES) -o practicefs `pkg-config fuse3 --cflags --libs`
	echo 'To Mount: ./practicefs -f [mount point]'

clean:
	rm practicefs