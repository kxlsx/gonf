# **gonf**

The flag parser generator.

It utilizes a simple syntax to generate command line
flag parsers written in C.

## Building

In order to build *gonf* using the provided Makefile,
you need:
```bash
gcc            # v10-2.1
flex           # v2.6.4
xxd            # v1.10
```

Any version of these programs should work, but I have written
the ones I have been using just in case.

To build/debug use:
```bash
make           # build with debug flags
make release   # build with release flags
make run       # build and run
make clean     # clean the output directory
```

## Usage

### Basic usage:

```text
gonf [FLAGS]... [FILES]...
```

The list of all available flags can be seen by
using the `--help` flag.

### Behavior:

The program tries to compile the provided files 
(reads from stdin if none are present)
into a C source file containing the generated parser.

The default output file name is `'gonf.c'`, but it can be changed with the `--output` flag.

Additionaly, one can use the `--header-file` option, 
in order to generate a C header file for the generated parser.

## Syntax

### Description:

The syntax can be quickly summarized as:
```text
IDENTIFIER: -SHORTNAME --LONGNAME "DESCRIPTION" = "DEFAULT_VALUE";
...
```
where:
* `IDENTIFIER` is a alphanumeric string.
* `SHORTNAME` is any ASCII character except `';"=-'\`.
* `LONGNAME` is a `SHORTNAME` followed by any string of characters except `';"='\`.
* `DESCRIPTION` is any string of characters except `NUL`.
* `DEFAULT_VALUE` is any string of characters except `NUL` *(has to be preceded by `'='`)*.

and where every part is optional except `SHORTNAME` or `LONGNAME` 
*(at least one must be present)* and a `';'` separator.

...or expressed in [Backus-Naur][bakus] form:
```html
<FLAGSPEC>    ::= <FLAG> <FLAGSPEC> | <FLAG>
<FLAG>        ::= <IDENTIFIER> <NAME> <DESCRIPTION> <VALUE> <SEPARATOR>
<IDENTIFIER>  ::= <TEXT> ":" | ""
<NAME>        ::= <SHORTNAME> <LONGNAME> | <SHORTNAME> | <LONGNAME>
<SHORTNAME>   ::= "-" <CHAR>
<LONGNAME>    ::= "--" <TEXT>
<DESCRIPTION> ::= <STRING> | ""
<VALUE>       ::= "=" <STRING> | "=" | ""
<SEPARATOR>   ::= ";"

<TEXT>   ::= <CHAR> <TEXT> | <CHAR>
<CHAR>   ::= ; any printable, non-whitespace char, except ';"=-'.
<STRING> ::= ; any string of characters (except NUL) enclosed in double quotes
```

The `IDENTIFIER`, `SHORTNAME` and `LONGNAME` **must** be unique.

C-style comments can also be used:
 - `//` for single-line, 
 - `/* */` for multiline.

Whitespace outside double quotes is completely ignored.

### Writing a gonf spec file:

Let's say you want to generate a flag parser recognizing the flags:
 1. `'help'`, abbreviated by `'h'`, described as `'Print help'`.
 2. `'o'`, accepting some value.
 3. `'skrzat'`, described as `'psotnik'`, accepting a value that's initially set as `'churbo'`.

A representant gonf file would then look like this:
```
-h --help "Print Help";
-o =;
--skrzat "psotnik" = "churbo";
```

Additionally, in order have easier access to the flags in code, 
an identifier can be specified:
```
HELP: -h --help "Print Help";
OUTPUT: -o =;
MCFLUNGUS: --skrzat "psotnik" = "churbo";
```

Finally, you can add some comments:
```
HELP: -h --help "Print Help"; // single-line comment
OUTPUT: -o =;
/* mutliline
            comment
            */
MCFLUNGUS: --skrzat "psotnik" = "churbo";
```

## Generated parser

### Contents:

The generated parser is a C source file containing various functions allowing one to:
 * Check how many times a flag has appeared.
 * Read the flag's shortname, longname or description.
 * Read the flag's default and set value *(if it has one)*.
 * Obtain an array of every passed non-flag argument.
  
Provided are also functions allowing 
one to handle errors encountered when parsing.

An optional header file can be generated, 
containing declarations of every function and macro, 
as well as their descriptions.

### Basic usage:

The main parser function is called `gonfparse`. 
It scans the provided command line arguments for the
specified flags, generating errors when:
 * an unrecognized flag has been encountered.
 * a value has been passed to a flag not accepting a value.
 * no value has been provided for a value flag with no specified default value.

and returning an array of non-flag arguments.

Every flag is represented by `gonflag` struct containing fields
such as:
```c
char shortname;
char *longname;
char *description;
bool is_value;
char *default_value;
char *value;
gonfc_t count;
```

In order to then access a specific flag, 
one needs to use the `gonflag_get` functions.
Specific flags can be obtained by passing their:
 * index *(easily available with the `GONFLAG_INDEX` macro, 
 when an identifier has been specified)*
 * shortname *(`gonflag_get_by_short`)*
 * longname *(`gonflag_get_by_long`)*

Additional documentation as well as helper macros 
are provided in the *(optional)* generated header file.

### Example
```c
/* include the header file */
#include <gonf.h>

char **args;
/* parse flags */
args = gonfparse(argc, argv);
/* check for errors */
if(gonferror() != GONFOK){
    free(args);
    gonferror_print();
    return -1;
}

struct gonflag *flag;

/* Print a flag's name & description. */
flag = gonflag_get(GONFLAG_HELP);
printf("%c\n", flag->shortname);
printf("%s\n", flag->longname);
printf("%s\n", flag->description);

/* Print a flag's value. */
flag = gonflag_get(GONFLAG_INDEX(OUTPUT));
if(flag->count)
    printf("%s\n", flag->value);

/* Print a flag's value using helper macros
 * (will be set to default value if none was provided).
 */
if(gonflag_is_present(GONFLAG_MCFLUNGUS))
    printf("%s\n", gonflag_get_field(GONFLAG_MCFLUNGUS, value));
```

## Contributing

I appreciate any bug report/contribution/criticism.

[bakus]:https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form