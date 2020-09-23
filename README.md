# 8086
8086 (subset) emulator, tightly-coded


Files from my two answers to this code challenge: 
http://codegolf.stackexchange.com/a/52902/2381
http://codegolf.stackexchange.com/a/9065/2381

Recently modified to launch into a Forth interpreter.
The interpreter is written in C/Assembly/Pre-compiled Forth.
The assembly is performed by the preprocessor and the `forth()`
function acts as a bootloader.

It prints characters by passing through a unicode translation that
simulates the IBM code page 437 font.
