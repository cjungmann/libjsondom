# JSON DOM Library

JSON (**J**ava**S**cript **O**bject **N**otation) DOM (**D**ocument **O**bject **M**odel)

A simple library developed for a Bash-builtin JSON tool.

## Note

This project is under development, so this page is wrong.  Parts
of this README talk about things that are planned but not yet done.
Some ideas, implemented or not, may be discarded without warning.

## Introduction

This simple library can parse a JSON document to build a memory
tree of its contents.  Each node has pointers to its neighbors,
so any node can reach any other node (in the tree).

In the interest of simplicity, there will be a man page that
documents an API treating the node as an undocumented block of
memory on which certain operations can be done.  For more
control, use [doxygen][doxygen] to generate the developer's
documentation and poke around.  It shouldn't be too hard to
understand.

## Getting the Library

There are only a few steps for building the project, clone,
configure, make, and install:

### Build

~~~sh
git clone https://www.github.com/cjungmann/jsondom.git
cd jsondom
./configure
make
~~~

### Install

Install the project with `sudo make install`.  This will
put the libraries into */usr/local/lib* and the man
page into */usr/local/man/man3*

### Uninstall

You can remove all installed traces of the project with
`sudo make uninstall`.


### Testing

The project includes a test source file, *test_basic.c*.  Build
the test program with:

~~~sh
make test
~~~

which will compile any source file that begins with *test_* into
an executable file named with the suffix of the filename.  Thus,
*test_basic.c* will create an executable file named *basic*.
There probably won't be any more *test_* files, but a developer
could write their own for their own purposes.

## Test Cases

In the interest of conforming to the [JSON standard][jsondef], there
are several small JSON documents in the **json_files** directory.
Files with a **bad_** prefix are invalid documents that should trigger
parsing warnings that say what and where it went wrong.  Files that
start with **good_** should parse without error.

The test executable, **basic**, reads **json_files/test.list**
for the list of files to parse.  Lines that begin with **\#**
will be ignored, the other lines should be file names to include
in a test run.

**json_files/test.list** includes all the JSON files in **json_files**
as well as several commented-out file names that can be downloaded
from [Microsoft][ms_dummy_data] for further testing.

When **basic** runs, each failed parsing will indicate the failure
and offer the choice to quit immediately or to continue on to the
next test.

### Future Test Cases

The article, [Parsing JSON is a Minefield][minefield] describes the
challenges of parsing JSON and, more importantly to my eyes, includes
a companion [test cases repository][minefield-tests] that *should*
eventually be used to measure the performance of this *jsondom*
library.

## REFERENCES

- [JSON Definition][jsondef]
- [Dummy JSON data for Microsoft Edge][ms_dummy_data]
- [Model implementation of JSON parser][libjson]
- [JSON is a Minefield][minefield]
- [Minefield test data][minefield-tests]

[jsondef]:          https://www.json.org/json-en.html
[ms_dummy_data]:    https://microsoftedge.github.io/Demos/json-dummy-data/
[libjson]:          https://github.com/vincenthz/libjson
[doxygen]:          https://www.doxygen.nl/
[minefield]:        https://seriot.ch/software/parsing_json.html
[minefield-tests]:  https://github.com/nst/JSONTestSuite