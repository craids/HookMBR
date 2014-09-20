# HookMBR
Template code for hooking the Master Boot Record. Does not work on UEFI boot.

## hook.c
Searches for a code cave and overwrites it with the trampoline code.

## trampoline.S
BIOS executes shellcode defined here. Success message is printed multiple times.
Original boot sector is backed up to 0x8000 before execution and then copied back to 0x7c00.  
Normal boot process continues after trampoline is executed.
