## fswatch.c (Mac OSX)

### I have made modifications to the original program

My version does not fork a process. It can writes out lines on stdout that provide
information about changed files. This is a lot more precise and informative. 

It also can quit when first chnage is found so that you can use this is bash scripts.

## Original readme follows. 

This is a small program using the Mac OS X FSEvents API to monitor a directory.
When an event about any change to that directory is received, the specified
shell command is executed by `/bin/bash`.

If you're on GNU/Linux,
[inotifywatch](http://linux.die.net/man/1/inotifywatch) (part of the
`inotify-tools` package on most distributions) provides similar
functionality.

### Compile

You need to be on Mac OS X 10.5 or higher with Developer Tools
installed.  Then, run `make`.  Copy the resulting `fswatch` binary to
a directory on your `$PATH`.

### Basic Usage

    usage: ./fswatch -h -q -s -p /some/directory[:/some/otherdirectory:...]

### About

This code was adapted from the example program in the
[FSEvents API documentation](https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/FSEvents_ProgGuide/FSEvents_ProgGuide.pdf).
