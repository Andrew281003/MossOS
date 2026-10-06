#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load(uint32_t ptr);
extern void irq12(void);
extern void irq1(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    // Clear out the entire IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Register mouse interrupt (IRQ12 -> INT 44)
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    idt_set_gate(44, (uint32_t)irq12, cs, 0x8E);

    // Register keyboard interrupt (IRQ1 -> INT 33)
    idt_set_gate(33, (uint32_t)irq1, cs, 0x8E);

    // Load IDT
    idt_load((uint32_t)&idtp);
}
