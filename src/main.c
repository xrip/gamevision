#pragma GCC optimize("Ofast")
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include <pico/sync.h>
#include <hardware/structs/vreg_and_chip_reset.h>
#include <hardware/clocks.h>

#include "sound/minigb_apu.h"
#include "peanut_gb.h"

#include "watara_rom.h"
#include "gb_cart.h"


// Address Bus (A0 - A16)
#define ADDR_MASK 0x1FFFF  // Bits 0-16

// Data Bus (D0 - D7)
#define DATA_MASK (0xFF << 17)

// /RD
#define RD_PIN  29
#define READ_MASK (1u << RD_PIN)

#define BITMAP_OFFEST 0x4000
#define AUDIO_OFFSET 0x5E00
#define CONTROL_OFFEST 0x6F00

struct semaphore vga_start_semaphore;
struct gb_s gb;
volatile uint8_t control = 0xFF;

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t __not_in_flash_func(gb_rom_read)(struct gb_s *gb, const uint_fast32_t addr) {
    return gb_cart[addr];
}
static uint8_t cart_ram[32768];
/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t __not_in_flash_func(gb_cart_ram_read)(struct gb_s *gb, const uint_fast32_t addr) {
    return cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void __not_in_flash_func(gb_cart_ram_write)(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val) {
    cart_ram[addr] = val;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr) {
    const char *gb_err_str[4] = {
        "UNKNOWN",
        "INVALID OPCODE",
        "INVALID READ",
        "INVALID WRITE"
    };
}

/**
 * Draws scanline into framebuffer.
 */
void __not_in_flash_func(lcd_draw_line)(struct gb_s *gb, const uint8_t pixels[160],
                   const uint_fast8_t line) {
    for (unsigned int x = 0; x < LCD_WIDTH; x += 4)
        rom[BITMAP_OFFEST + line * 48 + x / 4] =
                ((pixels[x + 3] & 3) << 6) | ((pixels[x + 2] & 3) << 4) | ((pixels[x + 1] & 3) << 2) | ((pixels[x] & 3));
}


volatile uint8_t gamepad_state = 0xff;
static inline uint8_t supervision_to_gameboy(uint8_t state) {
    // Output layout (bit positions):
    // 0:A, 1:B, 2:SELECT, 3:START, 4:RIGHT, 5:LEFT, 6:UP, 7:DOWN
    return
        (state & 0x20) >> 5 |  // A      (bit5 -> bit0)
        (state & 0x10) >> 3 |  // B      (bit4 -> bit1)
        (state & 0x40) >> 4 |  // SELECT (bit6 -> bit2)
        (state & 0x80) >> 4 |  // START  (bit7 -> bit3)
        (state & 0x01) << 4 |  // RIGHT  (bit0 -> bit4)
        (state & 0x02) << 4 |  // LEFT   (bit1 -> bit5)
        (state & 0x08) << 3 |  // UP     (bit3 -> bit6)
        (state & 0x04) << 5;   // DOWN   (bit2 -> bit7)
}

void __time_critical_func(second_core)() {
    int16_t audio_stream[AUDIO_BUFFER_SIZE_BYTES];
    sem_acquire_blocking(&vga_start_semaphore);


    /* Initialise GB context. */
    const enum gb_init_error_e ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
                                             &gb_cart_ram_write, &gb_error, NULL);


    gb_init_lcd(&gb, &lcd_draw_line);

    unsigned int frame_cnt = 0;
    unsigned int frame_timer_start = 0;


    // Initialize audio emulation
    audio_init();

    while (1) {
        gb.direct.joypad = supervision_to_gameboy(gamepad_state);

        gb_run_frame(&gb);

        audio_callback(NULL, audio_stream, AUDIO_BUFFER_SIZE_BYTES);
        // 738*2 байт 8итный буфер для ватары, сразу за видео буфером
        for (int i = 0; i < AUDIO_SAMPLES*2; i+=4){
            rom[(i >> 2) + AUDIO_OFFSET] = (audio_stream[i] >> 12) | (audio_stream[i+2] >> 8);
        }

        if (true) {
            if (++frame_cnt == 6) {
                while (time_us_64() - frame_timer_start < 16666 * 6); // 60 Hz
                frame_timer_start = time_us_64();
                frame_cnt = 0;
            }
        }

        tight_loop_contents();
    }
}



void __time_critical_func(handle_bus)() {
    uint8_t cntr = 0;
    while (true) {
        while (gpio_get_all() & READ_MASK);

        const uint32_t address = gpio_get_all() & 0x7fff ;

        if (address >= CONTROL_OFFEST && address <= (CONTROL_OFFEST+0xFF)) {
            gamepad_state = (address - CONTROL_OFFEST) & 0xff;
        } else {
            gpio_set_dir_out_masked(DATA_MASK);
            gpio_put_all(rom[address] << 17);
            gpio_set_dir_in_masked(DATA_MASK);
        }
    }
}

int main() {
    // Set the system clock speed.
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_us(35);
    set_sys_clock_hz(400 * MHZ, true); // 100x of Watara Supervision clock speed

    // Initialize all input pins at once
    gpio_init_mask(ADDR_MASK | DATA_MASK | READ_MASK);
    gpio_set_dir_in_masked(ADDR_MASK | DATA_MASK | READ_MASK);

    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(second_core);
    sem_release(&vga_start_semaphore);
    for (int i = 0; i< 0xff; i++ ) rom[0x300 + i] = i;
    handle_bus();
    return 0;
}
