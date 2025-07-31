CC=cc
CFLAGS=-Wall -Wextra -Wpedantic -march=native -Iinclude/ -g -O0

# fix for mingw errors
ifeq "$(CC)" "x86_64-w64-mingw32-gcc"
	CFLAGS+=-mwindows -L -lshlwapi
endif

LDFLAGS=

OUTFILE=miscounts-legacy

SRCDIR=src/
INCDIR=include/
DEPSDIR=deps/

# Object files
OBJS = a.o b.o c.o d.o miscount.o

# Build dependencies
buildDeps: a.o b.o c.o d.o miscount.o

a.o: $(DEPSDIR)strdup/strdup.c
	$(CC) -c $(CFLAGS) $< -o $@

b.o: $(DEPSDIR)str-starts-with/str-starts-with.c
	$(CC) -c $(CFLAGS) $< -o $@

c.o: $(DEPSDIR)str-ends-with/str-ends-with.c
	$(CC) -c $(CFLAGS) $< -o $@

d.o: $(DEPSDIR)path-join/path-join.c
	$(CC) -c $(CFLAGS) $< -o $@

miscount.o: $(SRCDIR)miscount.c
	$(CC) -c $(CFLAGS) $< -o $@

# Build main executable
buildMain: buildDeps src/main.c
	$(CC) -o $(OUTFILE) $(CFLAGS) src/main.c $(OBJS)

# Install
install: $(OUTFILE)
	sudo install -m775 $(OUTFILE) /usr/local/bin/$(OUTFILE)

# Clean up
clean:
	rm -f $(OBJS) $(OUTFILE)
