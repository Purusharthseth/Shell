# 🐚 Custom Shell in C++

A minimal shell implementation in C++ supporting basic built-in commands like `cd`, `echo`, `type`, `exit`, and `pwd`, along with the ability to execute external commands using `PATH`.

## 🚀 Features

- Built-in commands:
  - `echo`
  - `type`
  - `cd`
  - `pwd`
  - `exit`
- Support for multi-line input using `\`
- Quoting and escaping similar to typical UNIX shells
- Execution of external commands via `$PATH`

## 🛠️ Build Instructions

### Prerequisites
- A C++17 or later compiler

### Using `g++`
```bash
g++ -std=c++17 -o my_shell src/main.cpp
./my_shell

💻 Usage
Once compiled, you can use it like a shell:

sh
Copy
Edit
$ echo Hello, world!
Hello, world!

$ pwd
/home/user

$ cd /tmp
$ pwd
/tmp

$ type echo
echo is a shell builtin

$ type ls
ls is /bin/ls

$ ./nonexistent
./nonexistent: command not found

Supports multi-line input:

sh
Copy
Edit
$ echo Hello \
> world
Hello world
