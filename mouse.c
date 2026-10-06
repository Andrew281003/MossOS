#include "mouse.h"
#include "io.h"
#include "pic.h"

int mouse_x = 400;
int mouse_y = 300;
int mouse_b = 0;

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

// Wait until the controller is ready for us to send a command
static void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

// Write to mouse
static void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a_write);
}

// Read from mouse
static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    uint8_t status;

    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // Set defaults
    mouse_write(0xF6);
    mouse_read();

    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();
}

void mouse_handler(void) {
    uint8_t status = inb(0x64);
    if (!(status & 1)) {
        pic_send_eoi(12);
        return;
    }
    
    if (!(status & 0x20)) {
        // Not a mouse interrupt
        pic_send_eoi(12);
        return;
    }

    mouse_byte[mouse_cycle++] = inb(0x60);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        
        // Parse the packet
        int dx = mouse_byte[1];
        int dy = mouse_byte[2];

        // Sign extension
        if (mouse_byte[0] & 0x10) dx |= 0xFFFFFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFFFFFF00;

        mouse_x += dx;
        mouse_y -= dy; // Y is flipped

        // Clamp
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x >= 800) mouse_x = 799;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y >= 600) mouse_y = 599;

        mouse_b = mouse_byte[0] & 0x07;
    }

    pic_send_eoi(12);
}
