#pragma GCC optimize("Ofast")
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include <pico/sync.h>
#include <hardware/structs/vreg_and_chip_reset.h>

#include "sound/minigb_apu.h"
#include "peanut_gb.h"

#include "watara_rom.h"
#include "gb_cart.h"


// Pin Definitions.
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5
#define A6 6
#define A7 7
#define A8 8
#define A9 9
#define A10 10
#define A11 11
#define A12 12
#define A13 13
#define A14 14
#define A15 15
#define A16 16

#define D0 27
#define D1 26
#define D2 22
#define D3 21
#define D4 20
#define D5 19
#define D6 18
#define D7 17

#define NWR 28

#define BITMAP_OFFEST 0x4000

#define DATAOFFSETLOW 17
#define DATAOFFSETHIGH 20

// Bit masks
#define DATAMASK 0b01100011111100000000000000000
#define READMASK 0b10000000000000000000000000000

struct semaphore vga_start_semaphore;
struct gb_s gb;

uint8_t control;

void initGPIO() {
    // Set pin directions.
    gpio_init(A0);
    gpio_set_dir(A0, GPIO_IN);
    gpio_init(A1);
    gpio_set_dir(A1, GPIO_IN);
    gpio_init(A2);
    gpio_set_dir(A2, GPIO_IN);
    gpio_init(A3);
    gpio_set_dir(A3, GPIO_IN);
    gpio_init(A4);
    gpio_set_dir(A4, GPIO_IN);
    gpio_init(A5);
    gpio_set_dir(A5, GPIO_IN);
    gpio_init(A6);
    gpio_set_dir(A6, GPIO_IN);
    gpio_init(A7);
    gpio_set_dir(A7, GPIO_IN);
    gpio_init(A8);
    gpio_set_dir(A8, GPIO_IN);
    gpio_init(A9);
    gpio_set_dir(A9, GPIO_IN);
    gpio_init(A10);
    gpio_set_dir(A10, GPIO_IN);
    gpio_init(A11);
    gpio_set_dir(A11, GPIO_IN);
    gpio_init(A12);
    gpio_set_dir(A12, GPIO_IN);
    gpio_init(A13);
    gpio_set_dir(A13, GPIO_IN);
    gpio_init(A14);
    gpio_set_dir(A14, GPIO_IN);
    gpio_init(A15);
    gpio_set_dir(A15, GPIO_IN);
    gpio_init(A16);
    gpio_set_dir(A16, GPIO_IN);

    gpio_init(NWR);
    gpio_set_dir(NWR, GPIO_IN);

    // Initially, set the pins to IN
    gpio_init(D0);
    gpio_set_dir(D0, GPIO_IN);
    gpio_init(D1);
    gpio_set_dir(D1, GPIO_IN);
    gpio_init(D2);
    gpio_set_dir(D2, GPIO_IN);
    gpio_init(D3);
    gpio_set_dir(D3, GPIO_IN);
    gpio_init(D4);
    gpio_set_dir(D4, GPIO_IN);
    gpio_init(D5);
    gpio_set_dir(D5, GPIO_IN);
    gpio_init(D6);
    gpio_set_dir(D6, GPIO_IN);
    gpio_init(D7);
    gpio_set_dir(D7, GPIO_IN);
}

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t __not_in_flash_func(gb_rom_read)(struct gb_s *gb, const uint_fast32_t addr) {
    return gb_cart[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t __not_in_flash_func(gb_cart_ram_read)(struct gb_s *gb, const uint_fast32_t addr) {
    //  return cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void __not_in_flash_func(gb_cart_ram_write)(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val) {
    //    cart_ram[addr] = val;
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
                (pixels[x + 3] & 3) | ((pixels[x + 2] & 3) << 2) | ((pixels[x + 1] & 3) << 4) | ((pixels[x] & 3) << 6);
}

int16_t audio_stream[AUDIO_BUFFER_SIZE_BYTES] = { 0 };

void __time_critical_func(second_core)() {
    sem_acquire_blocking(&vga_start_semaphore);


    /* Initialise GB context. */
    const enum gb_init_error_e ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
                                             &gb_cart_ram_write, &gb_error, NULL);

    while (ret != GB_INIT_NO_ERROR) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
    }

    gb_init_lcd(&gb, &lcd_draw_line);

    int frame_cnt = 0;
    int frame_timer_start = 0;


    // Initialize audio emulation
    audio_init();

    while (1) {
        gb_run_frame(&gb);

        audio_callback(NULL, audio_stream, AUDIO_BUFFER_SIZE_BYTES);

        // 738*2 байт 8итный буфер для ватары, сразу за видео буфером
        for (int i = 0; i < AUDIO_SAMPLES*2; i+=4){
            rom[(i / 4)+(BITMAP_OFFEST+160*48)] = ((audio_stream[i]) >> 12) << 4 | ((audio_stream[i+2]) >> 12);
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

static inline bool overclock() {
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(10);
    return set_sys_clock_khz(378 * 1000, true);
}

int __time_critical_func(main)() {
    overclock();

    initGPIO();

    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(second_core);
    sem_release(&vga_start_semaphore);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    rom[0x70FF]=0;
    while (1) {
        const uint32_t data = gpio_get_all();
        const uint32_t oe = data & READMASK;

        (oe ? gpio_set_dir_in_masked(DATAMASK) : gpio_set_dir_out_masked(DATAMASK));
        if (false == oe) {
            const uint32_t address = data & ADDRMASK;
            if (address >= 0x7000 && address <= 0x70FF) {
                control = address & 0xFF;
                gb.direct.joypad = ((control >> 4)  & 0xF) | ((control << 4)  & 0x30) |  ((control << 5)  & 0x80) |  ((control << 3)  & 0x40);
                //gb.direct.joypad = ((control >> 4)  & 0xF) | ((control << 4)  & 0x30) |  ((control << 5)  & 0x80) |  ((control << 3)  & 0x40);
                //gb.direct.joypad = temp;
                //gb.direct.joypad_bits.start = 1;
            }

            const uint8_t romByte = rom[address];

            gpio_put_all((romByte & 0b00111111) << DATAOFFSETLOW |
                                        (romByte & 0b11000000) << DATAOFFSETHIGH);
        }
    }

    return 0;
}
