# Mini-Operating-System
A C-based operating system simulation project built in two phases to mimic job control, memory paging, instruction handling, and interrupt management.

## Project Overview
- Simulates memory with 300 blocks (4 chars each)
- Implements instruction parsing (`GD`, `PD`, `LR`, `SR`, `CR`, `BT`, `H`)
- Simulates page tables, valid/invalid page faults
- Handles TTL (Total Time Limit) & TLL (Total Line Limit) for jobs
- Provides detailed error handling and output logging

## Files
- `phase1.c` – Initial version with basic instruction simulation  
- `phase2.c` – Advanced simulation with paging and PCB-level job management  
- `input.txt` – Sample input file with job cards (`$AMJ`, `$DTA`, `$END`)  
- `output.txt` – Output generated after execution

## Tech Used
- C
- File I/O
- Memory simulation
- Operating System fundamentals

## How to Run
```bash
gcc phase2.c -o os_sim
./os_sim
```

Author
Himanshu Khandelwal
GitHub: https://github.com/HimanshuKK08
