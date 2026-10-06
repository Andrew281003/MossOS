.global idt_load
idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.global irq12
.extern mouse_handler
irq12:
    pusha
    cld
    call mouse_handler
    popa
    iret

.global irq1
.extern keyboard_handler
irq1:
    pusha
    cld
    call keyboard_handler
    popa
    iret
