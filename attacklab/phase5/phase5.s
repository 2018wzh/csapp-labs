.section .text
.global main
gadget1:
    movq %rsp, %rax
    ret
gadget2:
    movq %rax, %rdi
    ret
gadget3:
    popq %rax
    ret
gadget4:
    movl %eax, %edx
    ret
gadget5:
    movl %edx, %ecx
    ret
gadget6:
    movl %ecx, %esi
    ret
gadget7:
    movq %rax, %rdi
    ret
