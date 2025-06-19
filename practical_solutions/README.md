# Practical Solutions

This folder contains my C code solutions for the practical exercises from the exams.

## Compilation

It includes a `Makefile` to compile the C files. To compile all `.c` files in this directory, just run:

```bash
make
```

This will create a `bin` directory and place all the compiled executables there.

To clean up and remove the `bin` directory and all compiled files, run:

```bash
make clean
```

## Running an Exercise

To run a specific exercise, use the `run` command and specify the exercise name (which is the name of its folder). For example, if you have an exercise in a folder named `ex1`, you can run it with:

```bash
make run EXERCISE=ex1
``` 