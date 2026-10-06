#include "pic.h"
#include "io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

void pic_remap(void) {
    unsigned char a1, a2;

    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // Start initialization sequence in cascade mode
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();

    // ICW2: Vector offset
    outb(PIC1_DATA, 0x20); io_wait(); // Master PIC vector offset (32)
    outb(PIC2_DATA, 0x28); io_wait(); // Slave PIC vector offset (40)

    // ICW3: Cascade setup
    outb(PIC1_DATA, 4); io_wait(); // Tell Master there is a slave PIC at IRQ2
    outb(PIC2_DATA, 2); io_wait(); // Tell Slave its cascade identity

    // ICW4: Environment info
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    // Restore masks (enable IRQ1 keyboard, and IRQ2 cascade)
    outb(PIC1_DATA, ~((1 << 1) | (1 << 2))); 
    outb(PIC2_DATA, ~(1 << 4)); // IRQ12 is bit 4 of slave PIC
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}
