# TreePipe Project

## Overview
This project simulates a shell command (`TreePipe`) that builds a binary process tree using **fork**, **execvp**, and **pipes** for interprocess communication.

## Files
- `treePipe.c` - Main program that creates and orchestrates the process tree.
- `pl.c` - Implements the left child process (performs addition).
- `pr.c` - Implements the right child process (performs multiplication).
- `s1.txt`, `s2.txt`, `s3.txt`, `s4.txt` - Sample outputs from test runs.
- `CS_307_PA1_SP25.pdf` - Assignment description.
- `CS_307_HW1_Report.pdf` - Project report explaining the implementation.

## How to Compile and Run
### Compilation
Compile the source files using `gcc`:
```bash
gcc -o treePipe treePipe.c
gcc -o left pl.c
gcc -o right pr.c
