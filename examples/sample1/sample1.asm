; This is a very simple example for getting started
; with FlowMatrix Query utilities. You don't have to
; compile this file as it only contains two move 
; instructions. If you wish to do so, you will need 
; as31 and nasm.
; Compile with following:
;   nasm -f elf64 sample1.asm  
;   ld -s -o sample1 sample1.o

section     .text
global      _start 
_start:
    mov rbx, rdx
    mov rcx, rbx
    
section     .data
