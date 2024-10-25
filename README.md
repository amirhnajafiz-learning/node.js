# safex - Application Sandbox Using ptrace

## Overview

`safex` is a Linux application that uses the `ptrace` system call to create a sandbox for executing applications safely with respect to file accesses. The primary purpose of `safex` is to enforce rules on file read and write operations while allowing users to run potentially unsafe applications in a controlled environment.

## Features

- **File Read Protection**: 
  - The application can read any file that does not appear in the user's `.safenoread` file located in the home directory.
  
- **File Write Redirection**: 
  - If an application attempts to open a file for writing, `safex` creates a temporary copy of that file. The application's access is redirected to this temporary copy to prevent failure due to denial of write access.
  
- **Automatic Child Process Tracing**: 
  - Automatically traces any child processes created by the application using `fork()`, `vfork()`, or `clone()`. This ensures that all subprocesses are also subjected to the same file access rules.

## Installation

1. **Clone the Repository** (if applicable):
   ```bash
   git clone <repository-url>
   cd safex
   ```

2. **Compile the Program**:
   ```bash
   gcc -o safex safex.c -lptrace
   ```

3. **Modify the `.safenoread` File**:
   - Create or modify the file `~/.safenoread` to specify any files that should not be readable by the applications run through `safex`.

## Usage

Run the `safex` program with the target application and its arguments:

```bash
sudo ./safex <application> [args...]
```

### Example

To run a hypothetical application `myapp` safely:

```bash
sudo ./safex ./myapp --option value
```

## File Structure

- `safex.c`: The main source code file implementing the sandbox functionality.

## Dependencies

- **Linux**: The program relies on the Linux kernel's `ptrace` mechanism.
- **gcc**: For compiling the C source code.

## Limitations

- The program currently focuses solely on file access safety and does not handle network access or other types of operations.
- The performance may be affected by the overhead introduced by `ptrace`, especially when dealing with applications that frequently fork child processes.
