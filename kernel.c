#include <stdint.h>
#include "multiboot.h"
#include "graphics.h"
#include "idt.h"
#include "pic.h"
#include "mouse.h"
#include "io.h"

void keyboard_handler(void) {
    // Read the scancode from the keyboard controller to clear the buffer
    inb(0x60);
    // Send End-Of-Interrupt to the Master PIC
    pic_send_eoi(1);
}

void kernel_main(uint32_t magic, struct multiboot_info* mbd) {
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        return; // Something went wrong, halt
    }

    // Initialize low level systems
    idt_init();
    pic_remap();
    mouse_init();
    
    // Enable interrupts
    asm volatile("sti");

    init_graphics(mbd);

    // Main event loop
    while (1) {
        // Draw the static desktop to the backbuffer
        draw_win95_desktop(mouse_x, mouse_y, mouse_b);
        
        // Draw the mouse on top
        draw_mouse_cursor(mouse_x, mouse_y);
        
        // Push the backbuffer to the screen
        swap_buffers();

        // Optional: wait a bit to avoid maxing out the CPU for no reason
        for (volatile int i = 0; i < 10000; i++);
    }
}
