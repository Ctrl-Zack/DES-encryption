# DES Encrypted Socket Communication in C++

A secure client-server communication application implemented in C++ featuring custom DES (Data Encryption Standard) encryption over network sockets.

## Features
- **Socket-based Communication:** Real-time data transfer between client and server.
- **Custom DES Encryption:** Protects transmitted payloads using the DES cryptographic algorithm.
- **Cross-Platform Support:** Configured via Makefile to support Linux (`g++`) and Android (`clang++`).
- **Modern C++ Standard:** Built using C++23.

---

## Prerequisites

Before building and running the project, make sure you have the following installed:
- **Make** (GNU Make)
- **C++ Compiler supporting C++23**:
  - Linux: `g++-13` (or compatible GCC)
  - Android (Termux): `clang++`

---

## Getting Started & Compilation

The project uses a flexible **Makefile** that automatically creates a `bin/` directory for compiled binaries and handles the `include/` directory automatically.

### 1. Compile the Server and Client

You can compile the specific application binaries using the provided Makefile targets:

```bash
make server
make client

```

### 2. Run the Server and Client

To run the server and client applications, use the following shortcuts (run them in separate terminal windows):

* **Start the Server:**
```bash
make run-server

```


* **Start the Client:**
```bash
make run-client

```



### 3. Compile a Specific Custom Source File

If you add other source files and want to compile a specific one on the fly:

```bash
make file=custom_file.cpp

```

### 4. Clean Build Files

To remove the `bin/` directory and all compiled binaries:

```bash
make clean

```

---

## Project Structure

```text
├── include/
│   ├── constant.hpp    # DES bit tables, permutation boxes, and constants
│   ├── des.hpp         # DES encryption/decryption algorithm logic
│   └── secure_io.hpp   # Socket communication helper utilities
├── bin/                # Compiled binaries directory (auto-generated)
├── server.cpp          # Application server entry point
├── client.cpp          # Application client entry point
├── Makefile            # Build configuration script
└── README.md           # Project documentation
```