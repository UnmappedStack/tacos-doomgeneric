#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"
#include <stdio.h>
#include <syscall.h>
#include <sys/mman.h>

typedef struct {
    uint64_t bpp;
    uint64_t pitch;
    uint32_t *ptr;
} Fb;

Fb fb = {0};

void get_fb_info(uint64_t *pitchbuf, uint64_t *bppbuf) {
    __syscall2(24, (size_t) pitchbuf, (size_t) bppbuf);
}

void DG_Init() {
    get_fb_info(&fb.pitch, &fb.bpp);
    int fb_fd = open("/dev/fb0", 0, 0);
    if (fb_fd < 0) {
        printf("Failed to open framebuffer device.\n");
        exit(-1);
    }
    fb.ptr = mmap(NULL, 0, 0, 0, fb_fd, 0);
    printf("fb ptr = %p, pitch = %zu, bpp = %zu\n", fb.ptr, fb.pitch, fb.bpp);
    close(fb_fd);
}

// yoink thanks dcraft
void DG_DrawFrame() {
    uint8_t *upto = (uint8_t*) fb.ptr;
    for (size_t y = 0; y < DOOMGENERIC_RESY; y++) {
        for (size_t x = 0; x < DOOMGENERIC_RESX; x++) {
            ((uint32_t*) upto)[x] = DG_ScreenBuffer[y * DOOMGENERIC_RESX + x];
        }
        upto += fb.pitch;
    }
}

void DG_SleepMs(uint32_t ms) {}

uint32_t DG_GetTicksMs() {
    static uint32_t ticks = 0;
    return ticks++;
}

int DG_GetKey(int* pressed, unsigned char* doomKey) {
    // TODO: this is a stub, always returns that there's no key press
    return 0;
}

void DG_SetWindowTitle(const char * title) {}

int main(int argc, char **argv) {
    printf("\x1b[2J");
    for (size_t i = 0; i < 8; i++) printf("\n");
    doomgeneric_Create(argc, argv);
    for (;;) {
        doomgeneric_Tick();
    }
    return 0;
}
