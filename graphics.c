#include "graphics.h"
#include "font.h"

static uint32_t* fb = 0;
static uint32_t backbuffer[800 * 600]; // 1.9MB static backbuffer
static int screen_width = 1200;
static int screen_height = 1080;
static int screen_pitch = 3200;

void init_graphics(struct multiboot_info* mbd) {
    if (mbd->flags & (1 << 12)) {
        fb = (uint32_t*)(uintptr_t)mbd->framebuffer_addr;
        screen_width = mbd->framebuffer_width;
        screen_height = mbd->framebuffer_height;
        screen_pitch = mbd->framebuffer_pitch;
    }
}

void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
    backbuffer[y * screen_width + x] = color;
}

void swap_buffers() {
    if (!fb) return;
    // Copy backbuffer to physical framebuffer
    for (int y = 0; y < screen_height; y++) {
        uint32_t* row_fb = (uint32_t*)((uint8_t*)fb + y * screen_pitch);
        uint32_t* row_bb = &backbuffer[y * screen_width];
        for (int x = 0; x < screen_width; x++) {
            row_fb[x] = row_bb[x];
        }
    }
}

void draw_mouse_cursor(int x, int y) {
    // Draw a simple white arrow with a black outline
    for (int i = 0; i < 12; i++) {
        put_pixel(x, y + i, COLOR_BLACK);
        put_pixel(x + i, y + i, COLOR_BLACK);
        for (int j = 1; j < i; j++) {
            put_pixel(x + j, y + i, COLOR_WHITE);
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

void draw_char(char c, int x, int y, uint32_t color, uint32_t bg_color) {
    if (c < 0x20 || c > 0x7E) return;
    int index = c - 0x20;
    
    for (int row = 0; row < 8; row++) {
        uint8_t row_data = font8x8[index][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (1 << (7 - col))) {
                put_pixel(x + col, y + row, color);
            } else {
                if (bg_color != 0xFFFFFFFF) { // hack to use 0xFFFFFFFF as transparent
                    put_pixel(x + col, y + row, bg_color);
                }
            }
        }
    }
}

void draw_string(const char* str, int x, int y, uint32_t color, uint32_t bg_color) {
    int cur_x = x;
    while (*str) {
        draw_char(*str, cur_x, y, color, bg_color);
        cur_x += 8;
        str++;
    }
}

// Helper to draw 3D borders
void draw_3d_border(int x, int y, int w, int h, int sunk) {
    uint32_t top_left = sunk ? COLOR_GRAY : COLOR_WHITE;
    uint32_t bottom_right = sunk ? COLOR_WHITE : COLOR_GRAY;
    uint32_t top_left_inner = sunk ? COLOR_BLACK : COLOR_SILVER;
    uint32_t bottom_right_inner = sunk ? COLOR_SILVER : COLOR_BLACK;

    // Outer border
    draw_rect(x, y, w - 1, 1, top_left);
    draw_rect(x, y, 1, h - 1, top_left);
    draw_rect(x + w - 1, y, 1, h, bottom_right);
    draw_rect(x, y + h - 1, w, 1, bottom_right);

    // Inner border
    draw_rect(x + 1, y + 1, w - 3, 1, top_left_inner);
    draw_rect(x + 1, y + 1, 1, h - 3, top_left_inner);
    draw_rect(x + w - 2, y + 1, 1, h - 1, bottom_right_inner);
    draw_rect(x + 1, y + h - 2, w - 2, 1, bottom_right_inner);
}

static int show_window = 1;
static int show_start_menu = 0;
static int prev_mouse_b = 0;

void draw_win95_desktop(int mx, int my, int mb) {
    if (!fb) return;

    // Detect clicks (rising edge of mouse_b bit 0)
    int left_clicked = (mb & 1) && !(prev_mouse_b & 1);
    prev_mouse_b = mb;

    // 1. Desktop Background
    draw_rect(0, 0, screen_width, screen_height, COLOR_TEAL);

    // 2. Taskbar (Bottom)
    int taskbar_height = 28;
    int taskbar_y = screen_height - taskbar_height;
    draw_rect(0, taskbar_y, screen_width, taskbar_height, COLOR_SILVER);
    draw_rect(0, taskbar_y, screen_width, 1, COLOR_WHITE);
    
    // 3. Start Button
    int btn_x = 2, btn_y = taskbar_y + 2, btn_w = 54, btn_h = 24;
    int start_pressed = (mb & 1) && (mx >= btn_x && mx <= btn_x + btn_w && my >= btn_y && my <= btn_y + btn_h);
    
    if (left_clicked && start_pressed) {
        show_start_menu = !show_start_menu;
    } else if (left_clicked && !start_pressed) {
        // Clicking elsewhere closes the start menu
        if (my < taskbar_y || mx > btn_x + btn_w) show_start_menu = 0;
    }

    draw_rect(btn_x, btn_y, btn_w, btn_h, COLOR_SILVER);
    draw_3d_border(btn_x, btn_y, btn_w, btn_h, start_pressed || show_start_menu); 
    draw_string("Start", btn_x + 8, btn_y + 8, COLOR_BLACK, COLOR_SILVER);

    // 4. Start Menu
    if (show_start_menu) {
        int menu_w = 150, menu_h = 200;
        int menu_x = 2;
        int menu_y = taskbar_y - menu_h;
        draw_rect(menu_x, menu_y, menu_w, menu_h, COLOR_SILVER);
        draw_3d_border(menu_x, menu_y, menu_w, menu_h, 0);
        
        // A blue stripe on the left
        draw_rect(menu_x + 2, menu_y + 2, 20, menu_h - 4, COLOR_DARK_BLUE);
        
        // Some fake items
        draw_string("Programs", menu_x + 30, menu_y + 20, COLOR_BLACK, COLOR_SILVER);
        draw_string("Documents", menu_x + 30, menu_y + 50, COLOR_BLACK, COLOR_SILVER);
        draw_string("Settings", menu_x + 30, menu_y + 80, COLOR_BLACK, COLOR_SILVER);
        
        // Separator
        draw_rect(menu_x + 25, menu_y + 160, menu_w - 30, 1, COLOR_GRAY);
        draw_rect(menu_x + 25, menu_y + 161, menu_w - 30, 1, COLOR_WHITE);
        
        draw_string("Shut Down...", menu_x + 30, menu_y + 175, COLOR_BLACK, COLOR_SILVER);
    }

    // 5. Mock Window
    if (show_window) {
        int win_w = 400, win_h = 250;
        int win_x = (screen_width - win_w) / 2;
        int win_y = (screen_height - win_h) / 2;

        draw_rect(win_x, win_y, win_w, win_h, COLOR_SILVER);
        draw_3d_border(win_x, win_y, win_w, win_h, 0);

        int title_h = 18;
        draw_rect(win_x + 3, win_y + 3, win_w - 6, title_h, COLOR_DARK_BLUE);
        draw_string("Welcome to MossOS", win_x + 6, win_y + 8, COLOR_WHITE, COLOR_DARK_BLUE);

        // Close Button
        int close_w = 16, close_h = 14;
        int close_x = win_x + win_w - close_w - 5;
        int close_y = win_y + 5;
        int close_pressed = (mb & 1) && (mx >= close_x && mx <= close_x + close_w && my >= close_y && my <= close_y + close_h);
        
        if (left_clicked && close_pressed) show_window = 0;

        draw_rect(close_x, close_y, close_w, close_h, COLOR_SILVER);
        draw_3d_border(close_x, close_y, close_w, close_h, close_pressed);
        draw_string("x", close_x + 4, close_y + 3, COLOR_BLACK, COLOR_SILVER);

        // Window Content Area
        int cx = win_x + 8, cy = win_y + title_h + 8;
        int cw = win_w - 16, ch = win_h - title_h - 16;
        draw_rect(cx, cy, cw, ch, COLOR_WHITE);
        draw_3d_border(cx, cy, cw, ch, 1);
        
        draw_string("Hello! You are running MossOS.", cx + 8, cy + 8, COLOR_BLACK, COLOR_WHITE);
        draw_string("The mouse works perfectly!", cx + 8, cy + 24, COLOR_BLACK, COLOR_WHITE);
        draw_string("Try clicking the Start button.", cx + 8, cy + 40, COLOR_BLACK, COLOR_WHITE);

        // Ok Button
        int ok_w = 60, ok_h = 24;
        int ok_x = win_x + (win_w - ok_w) / 2;
        int ok_y = win_y + win_h - 32;
        int ok_pressed = (mb & 1) && (mx >= ok_x && mx <= ok_x + ok_w && my >= ok_y && my <= ok_y + ok_h);
        
        if (left_clicked && ok_pressed) show_window = 0;

        draw_rect(ok_x, ok_y, ok_w, ok_h, COLOR_SILVER);
        draw_3d_border(ok_x, ok_y, ok_w, ok_h, ok_pressed);
        draw_string("OK", ok_x + 22, ok_y + 8, COLOR_BLACK, COLOR_SILVER);
    }
}
