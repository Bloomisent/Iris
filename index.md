# Iris

**Iris** is a small interpreted programming language written in C.

> **Current version:** v3.0.2
> **File extension:** `.iris`
> **License:** Apache License 2.0

Iris uses a lexer → parser → AST → visitor architecture. Source code is tokenized, parsed into an abstract syntax tree, and then executed by the visitor.

---

## Getting Started

An Iris program is stored in a file ending in `.iris`.

For example:

```iris
print("Hello, world!");
```

Run an Iris program by passing its path to the interpreter:

```text
iris.exe program.iris
```

The executable also supports:

```text
iris.exe --version
```

which prints the current interpreter version.

Iris currently does not include a Makefile or CMake configuration in the repository root, so building the interpreter requires compiling the C source files together with the required libraries.

---

# Basic Syntax

## Statements

Statements are separated using semicolons:

```iris
print("Hello");
print("World");
```

A semicolon can also terminate a single statement:

```iris
var x = 10;
```

Iris parses a program as a compound collection of statements.

---

# Variables

Variables are declared with `var`:

```iris
var x = 10;
var name = "Iris";
var enabled = true;
```

The value can be a number, string, boolean, table, dictionary, class, function result, or another expression.

Variables can be reassigned:

```iris
var x = 10;

x = 20;
```

The compound-assignment operators are also supported:

```iris
x += 5;
x -= 2;
x *= 3;
x /= 2;
```

The parser represents these as assignments and binary operations.

---

# Data Types

Iris currently supports the following principal runtime values:

| Type           | Example            |
| -------------- | ------------------ |
| Number         | `123`              |
| String         | `"hello"`          |
| Boolean        | `true` / `false`   |
| Table          | `[1, 2, 3]`        |
| Dictionary     | `["name": "Iris"]` |
| Class          | `MyClass`          |
| Class instance | `MyClass instance` |

Boolean literals are implemented as the identifiers `true` and `false`.

---

# Numbers

Numbers are parsed as floating-point values internally.

```iris
var x = 10;
var y = 2.5;

print(x + y);
```

Supported arithmetic operators include:

```text
+   addition
-   subtraction
*   multiplication
+=   self-addition
-=   self-subtraction
*=   self-multiplication
/=   self-division
%   modulo
^   exponentiation
```

Comparisons include:

```text
>
<
>=
<=
==
!=
```

These operators are implemented by the expression parser and visitor.

---

# Strings

Strings use double quotes:

```iris
var message = "Hello";
print(message);
```

Strings support concatenation with `+`:

```iris
var first = "Hello ";
var second = "world";

print(first + second);
```

The built-in `print` function also interprets `%n` inside strings as a newline:

```iris
print("Hello%nWorld");
```

Strings can contain escaped characters such as:

```text
\n
\t
\"
\\
```

The lexer handles these escape sequences when collecting strings.

---

# Comments

Comments are enclosed between `#` characters:

```iris
# This is a comment #
```

Comments may span multiple lines.

The lexer skips everything between the opening and closing `#`.

---

# Functions

Functions are declared using `func`:

```iris
func add(a, b) {
    return a + b;
}
```

They are called using parentheses:

```iris
var result = add(10, 20);
print(result);
```

Functions can have zero or more parameters:

```iris
func hello() {
    print("Hello");
}
```

```iris
func greet(name) {
    print("Hello " + name);
}
```

The interpreter checks the number of arguments passed to a function against the number declared by the function.

---

# Return

Functions return values with `return`:

```iris
func square(x) {
    return x * x;
}

var result = square(5);
```

A `return` causes the visitor to stop executing the current compound statement.

---

# Conditions

## `if`

```iris
if (x > 10) {
    print("Large");
}
```

## `ifelse`

Iris uses `ifelse` for an if/else statement:

```iris
ifelse (x > 10) {
    print("Large");
} else {
    print("Small");
}
```

The parser explicitly recognizes both `if` and `ifelse`.

Conditions consider numbers, strings, and booleans when determining truthiness.

---

# For Loops

The Iris `for` syntax contains three expressions separated by colons:

```iris
for (var i = 1: i <= 10: "++") {
    print(i);
}
```

The third expression specifies the step:

```text
"++"
```

increments the loop variable.

```text
"--"
```

decrements it.

For example:

```iris
for (var i = 10: i > 0: "--") {
    print(i);
}
```

The loop parser expects:

```text
for (initialisation : condition : step)
```

and the visitor specifically recognizes `"++"` and `"--"`.

---

# While Loops

While loops use:

```iris
while (condition) {
    ...
}
```

Example:

```iris
var x = 0;

while (x < 10) {
    print(x);
    x += 1;
}
```

The condition is reevaluated for every iteration.

---

# Tables

Tables are ordered collections.

Define a table with square brackets:

```iris
table numbers = [1, 2, 3, 4, 5];
```

A table can contain expressions:

```iris
table numbers = [1 + 1, 2 + 2, 3 + 3];
```

Tables can also be copied into another table definition:

```iris
table numbers2 = numbers;
```

The parser records table names separately so that later references can be recognized as table values.

## Table functions

Table operations are exposed through built-in functions:

```iris
table_get_index(table, value)
table_get_from_index(table, index)
table_set_index(table, index, value)
```

`table_get_from_index` uses **1-based indexing**.

For example:

```iris
var value = table_get_from_index(numbers, 1);
```

returns the first element.

`table_get_index` searches for a value and returns its index.

---

# Dictionaries

Dictionaries contain string keys and values.

```iris
dict person = [
    "name": "John",
    "age": 20
];
```

Dictionary entries use:

```text
"key": value
```

and entries are separated by commas.

Dictionary access and modification are provided through built-ins:

```iris
dict_get_from_index(dictionary, key)
dict_get_index(dictionary, value)
dict_set_index(dictionary, key, value)
```

For example:

```iris
var name = dict_get_from_index(person, "name");
```

---

# Member Access

Dictionary and class members can be accessed using a dot followed by a string:

```iris
person."name"
```

For example:

```iris
var name = person."name";
```

Members can also be assigned:

```iris
person."name" = "Alice";
```

The parser represents this operation as an `AST_DOT` node.

---

# Classes

Classes are declared with `class`:

```iris
class Player {
    "name": "Player",
    "score": 0
}
```

Class members use string keys.

A class can also contain a special `init` member:

```iris
class Player {
    "name": "Player",
    "init": func(self, name) {
        self."name" = name;
    }
}
```

The `init` member is parsed specially as a constructor-like function. Its first parameter is conventionally `self`.

---

# Class Instances

A class instance can be declared using:

```iris
Player player;
```

The parser treats the class name followed by another identifier as a class-instantiation expression.

Class instances can then be used with member access:

```iris
player."name"
```

---

# Checks

Iris provides a `checks` construct for comparing a value against multiple conditions.

Its structure is:

```iris
checks(value) {
    (condition): {
        ...
    },
    (condition): {
        ...
    }
}
```

Example:

```iris
checks(x) {
    (1): {
        print("one");
    },
    (2): {
        print("two");
    }
}
```

Each condition is compared with the value supplied to `checks`.

---

# Includes

Other `.iris` files can be included using:

```iris
include "library.iris";
```

The included path is resolved relative to the directory of the current Iris file.

Iris keeps track of included paths to prevent the same file from being included repeatedly, including circular include situations.

---

# Built-in Functions

Iris provides a collection of built-in functions.

## General

### `print`

Prints values:

```iris
print("Hello");
print(123);
print(true);
```

Strings containing `%n` can be used for newlines.

### `type`

Returns a string describing the type:

```iris
type(value)
```

Possible returned type names include:

```text
STRING
NUMBER
TABLE
CLASS
BOOL
```

### `toNumber`

Converts a string to a number:

```iris
toNumber("123")
```

### `toString`

Converts a number to a string:

```iris
toString(123)
```

---

## String Functions

### `charAt`

Returns a character at a 1-based index:

```iris
charAt("Hello", 1)
```

### `stredit`

Changes a character in a string:

```iris
stredit(string, index, value)
```

The replacement value is expected to contain one character.

### `ForEach`

Calls a named Iris function for each character in a string:

```iris
ForEach("Hello", "myFunction")
```

---

## File Functions

### `readFile`

Reads a file into a string:

```iris
var contents = readFile("file.txt");
```

### `writeFile`

Writes data to a file:

```iris
writeFile("file.txt", "Hello", false);
```

The third argument determines whether formatted output is used.

---

# Garbage Collection

Iris exposes garbage-collection functions:

```iris
gc();
gcStats();
```

`gc()` is intended to reclaim unused variable and function definitions from the global scope.

The implementation also tracks function-call depth and prevents collection while a function call is in progress.

---

# Windows Graphics

When Iris is compiled for Windows, it provides a small native windowing API.

## Create a window

```iris
windowCreate("My Iris Window", 800, 600);
```

## Clear the window

```iris
windowClear(0, 0, 0);
```

The arguments are RGB values.

## Draw a rectangle

```iris
windowDrawRect(x, y, width, height, r, g, b);
```

## Draw text

```iris
windowDrawText(x, y, text, r, g);
```

## Present the back buffer

```iris
windowPresent();
```

## Input and timing

```iris
windowGetTime();
windowShouldClose();
windowMouseX();
windowMouseY();
windowMousePressed();
```

## Close

```iris
windowClose();
```

These functions are implemented using the Windows API. When Iris is compiled on a non-Windows platform, the window functions report that windowing is only supported under `_WIN32`.

---

# Expression Precedence

Iris parses expressions in several levels:

1. Parenthesized expressions
2. Strings, numbers and identifiers
3. Member access / function calls
4. `*`, `/`, `%`, `^`
5. `+`, `-`
6. Comparisons:

   * `>`
   * `<`
   * `>=`
   * `<=`
   * `==`
   * `!=`

This hierarchy is implemented directly by the parser's expression functions.

---

# Program Structure

A typical Iris program can combine all of these features:

```iris
var name = "Iris";
var counter = 0;

func greet(person) {
    print("Hello " + person);
}

greet(name);

for (var i = 1: i <= 5: "++") {
    print(i);
}

if (counter == 0) {
    print("Counter is zero");
}
```

The interpreter processes the file by:

```text
.iris source
    ↓
Lexer
    ↓
Tokens
    ↓
Parser
    ↓
AST
    ↓
Visitor
    ↓
Execution
```

The lexer produces tokens containing a type, value, source position, line and column. The parser converts those tokens into AST nodes, and the visitor evaluates the resulting AST.

---

# Source Layout

The repository is organized around the interpreter implementation.

Important components include:

```text
src/
├── main.c
├── lexer.c
├── parser.c
├── visitor.c
├── AST.c
├── token.c
├── window.c
└── include/
    ├── AST.h
    ├── builtin.h
    ├── lexer.h
    ├── parser.h
    ├── scope.h
    ├── token.h
    ├── visitor.h
    └── window.h
```

`main.c` initializes the lexer and parser, parses the program, and then sends the resulting AST to the visitor for execution.

---

# Error Messages

Iris reports errors using messages such as:

```text
Tripped on undefined variable 'x' ...
Tripped on division by zero ...
Tripped on undefined method 'foo' ...
```

Parser errors generally use the line stored on the current token.

Runtime errors produced by the visitor and built-ins use the interpreter's global `current_line` value. Consequently, runtime error locations can differ from the actual source location of the AST node that caused the error.

---

# Quick Reference

| Feature             | Syntax                                    |
| ------------------- | ----------------------------------------- |
| Variable            | `var x = value;`                          |
| Assignment          | `x = value;`                              |
| Add assignment      | `x += value;`                             |
| Subtract assignment | `x -= value;`                             |
| Multiply assignment | `x *= value;`                             |
| Divide assignment   | `x /= value;`                             |
| Function            | `func name(args) { ... }`                 |
| Return              | `return value;`                           |
| If                  | `if (condition) { ... }`                  |
| If/else             | `ifelse (condition) { ... } else { ... }` |
| For                 | `for (start: condition: "++") { ... }`    |
| While               | `while (condition) { ... }`               |
| Table               | `table x = [a, b, c];`                    |
| Dictionary          | `dict x = ["key": value];`                |
| Class               | `class X { ... }`                         |
| Instance            | `X instance;`                             |
| Member access       | `object."key"`                            |
| Include             | `include "file.iris";`                    |
| Comment             | `# comment #`                             |

---

# Example Program

```iris
# Simple Iris program #

var name = "World";
var count = 5;

func greet(person) {
    print("Hello " + person);
}

greet(name);

for (var i = 1: i <= count: "++") {
    print(i);
}

if (count > 0) {
    print("Done");
}
```

This demonstrates variables, strings, functions, function calls, a `for` loop, comparisons, and an `if` statement.
