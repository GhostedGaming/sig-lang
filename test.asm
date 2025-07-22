global _start

section .data
    message db "Hello, world!", 0Ah, 0
    length equ $-message
    message2 db "Hello, world!2", 0Ah, 0
    length2 equ $-message2

section .text
_start:
    ; First write syscall
    mov rax, 1          ; sys_write
    mov rdi, 1          ; stdout
    mov rsi, message    ; first message buffer
    mov rdx, length     ; first message length
    syscall

    ; Second write syscall (rax and rdi still set)
    mov rax, 1
    mov rsi, message2   ; second message buffer
    mov rdx, length2    ; second message length
    syscall

    ; Exit syscall
    mov rax, 60         ; sys_exit
    mov rdi, 0          ; exit status
    syscall