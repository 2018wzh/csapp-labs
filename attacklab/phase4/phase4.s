.section .text
.global main
gadget1:
    pop %rax
    ret
gadget2:
    movq %rax, %rdi
    ret
