section .data
newline: db 0xA
str0: db "Hello world!", 0xA
x: db "Hello"
x_len: dd 5
str1: db "If", 0xA
str2: db "Elif", 0xA
str3: db "Hello", 0xA
str4: db "12334", 0xA

section .text
global _start

_start:
    call hello
; Variable x = Hello (type: string)
; If statement: 1 == 0
    mov eax, 1
    cmp eax, 0
    jne elif1
    mov rax, 1
    mov rdi, 1
    mov rsi, str1
    mov rdx, 3
    syscall
    jmp if_end0
elif1:
; Elif condition: 1 != 0
    mov eax, 1
    cmp eax, 0
    je if_end0
    mov rax, 1
    mov rdi, 1
    mov rsi, str2
    mov rdx, 5
    syscall
if_end0:
while_start2:
; While condition: 1
    mov rax, 1
    mov rdi, 1
    mov rsi, str3
    mov rdx, 6
    syscall
    jmp while_start2
while_end3:
    mov rax, 1
    mov rdi, 1
    mov rsi, str4
    mov rdx, 6
    syscall
    mov rax, 0
    mov rax, 60
    xor rdi, rdi
    syscall

; Function definitions

hello:
    push rbp
    mov rbp, rsp
    mov rax, 1
    mov rdi, 1
    mov rsi, str0
    mov rdx, 13
    syscall
    mov rsp, rbp
    pop rbp
    ret
