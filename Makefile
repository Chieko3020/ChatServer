.PHONY: all clean rebuild

# Default target: configure and build
all:
	mkdir -p build
	cd build && cmake .. && make

# Clean only build artifacts and binaries — source is never touched
clean:
	rm -rf build bin

# Full rebuild from scratch
rebuild: clean all
