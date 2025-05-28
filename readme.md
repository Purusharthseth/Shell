# 🐚 Custom Shell in C++

A minimal shell-like program written in C++, supporting basic built-in commands like `cd`, `echo`, `type`, `exit`, and `pwd`, with additional support for executing external commands found in the system's `$PATH`.

---

## 🚀 Features

- ✅ **Built-in Commands**:
  - `echo` — Print arguments to standard output
  - `type` — Identify whether a command is built-in or external
  - `cd` — Change the current working directory
  - `pwd` — Print the current working directory
  - `exit` — Exit the shell

- 🔁 **Multi-line Input** using backslash (`\`) continuation
- 🧠 **Smart Quoting** and escaping behavior similar to traditional UNIX shells
- 🧰 **External Command Execution** via `$PATH` lookup
- 📤 **I/O Redirection with support** for `>`, `<,` and `>>` using low-level system calls

### Prerequisites

- C++17 (or later) compatible compiler (e.g., `g++`, `clang++`)

---
## 🛠️ Build Instructions

### 🔧 Building with Makefile

##### ✅ To build the project:

```bash
make
```
##### ▶️ To run the shell:
```
./shell
```
##### 🧹 To clean the compiled files:
```bash
make clean
```



### 🔨 Compile Manually with `g++`

```bash
g++ -std=c++17 -o my_shell src/main.cpp
./shell
```
---
## 💻 Example Usage

```bash
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

```
