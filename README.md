# June

June is a optionally typed language with a VM written in C++ including support for C and Rust extensions and libraries.

A project roadmap can be found in [`ROADMAP.md`]('./ROADMAP.md')

## How it works

June uses a VM written in C++ to run bytecode provided by the included compiler.

2 main ways for running code use either the compiler (which loads and passes bytecode to the VM directly) or via compiled bytecode.

