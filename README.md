# miscounts-legacy
C rewrite of miscounts for legacy or stable systems

For more information, go to <https://github.com/gushtichudi/miscounts>

## Building
I've decided that I won't use my [build system](https://github.com/gushtichudi/miscounts) for this. Makefile is used.

### Dependencies
Moreover, this program involves the usage of the C package manager, [clib](https://github.com/clibs/clib). Copy the code-block and paste it into your terminal before you compile the program.

```
clib install stephenmathieson/path-join.c
```

## Known Bugs and oddities

### Write description in `$EDITOR`
Right now, I haven't been able to implement the feature to write description messages in another editor program. This is on the works.