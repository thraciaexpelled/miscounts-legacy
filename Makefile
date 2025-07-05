CC=clang
CFLAGS=-Wall -Wextra -Wpedantic -march=native -Iinclude/
LDFLAGS=

OUTFILE=miscounts-legacy

SRCDIR=src/
INCDIR=include/
DEPSDIR=deps/

# Object files
OBJS = a.o b.o c.o d.o

# Build dependencies
buildDeps: a.o b.o c.o d.o

a.o: $(DEPSDIR)strdup/strdup.c
	$(CC) -c $(CFLAGS) $< -o $@

b.o: $(DEPSDIR)str-starts-with/str-starts-with.c
	$(CC) -c $(CFLAGS) $< -o $@

c.o: $(DEPSDIR)str-ends-with/str-ends-with.c
	$(CC) -c $(CFLAGS) $< -o $@

d.o: $(DEPSDIR)path-join/path-join.c
	$(CC) -c $(CFLAGS) $< -o $@

# Build main executable
buildMain: buildDeps src/main.c
	$(CC) -o $(OUTFILE) $(CFLAGS) src/main.c $(OBJS)

# Clean up
clean:
	rm -f $(OBJS) $(OUTFILE)