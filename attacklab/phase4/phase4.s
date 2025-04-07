.section .text
.global main
gadget1:
    popq %rax
    ret
gadget2:
    movq %rax, %rdi
    ret
