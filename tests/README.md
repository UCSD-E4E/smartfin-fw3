# Overview
The folder `tests` contains tests monatomic tests for the scheduler in the
 `src` directory. There is a bash script to manage the tests at 
 `tests/run_gtests`.

# File Descriptions
## `tests/run_gtests` 
This should be used to run all test cases. This Bash script automates the build and testing process for the smartfin project using Google Test. It provides a command-line interface for managing compilation, testing, and graphical outputs in `tests/outputs/`.

## Features

- **Dynamic Compilation Options**: Compile with different settings like debug mode and using different compilers such as GCC or Clang.
- **Testing**: Run specific tests passed as arguments or all tests if none are specified.
- **Output Management**: Manage directories for output and disable graph generation if needed.

## Dependencies

- `md5sum` or `md5` for checksums to detect changes in files.
- `cmake` and `make` for building the project.
- `awk` for text processing within the script.
## Options

| Option              | Description                                                         |
|---------------------|---------------------------------------------------------------------|
| `-n`, `--no-graphs` | Disables the generation of graphs.|
| `-l`, `--list-tests`| Lists all the tests configured in the test file. |
| `-d`, `--output-dirs`| Enables output of directories where test results will be stored when run in conjuction with `-b` or `-n`|
| `-b`, `--build-only`| Compile the project without running tests.                         |
| `-c`, `--clang`     | Use the Clang compiler instead of GCC for compilation.            |
| `--debug`           | Compile in debug mode, allowing for usage of gdb/lldb          |
| `-t`, `--run-tests` | Run specific tests. This option should be followed by a list of space-separated test names to run.|

## Usage

```bash
./tests/run_gtests 
```