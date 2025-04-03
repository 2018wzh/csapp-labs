.section .text
.global main
main:
    mov $0x59b997fa,%rdi
    mov $0x4017ec,%rax
    jmp *%rax
