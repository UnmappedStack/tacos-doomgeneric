#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"
#include <stdio.h>
#include <TacOS.h>
#include <syscall.h>
#include <sys/mman.h>

typedef struct {
    int fd;
    uint64_t bpp;
    uint64_t pitch;
    uint32_t *ptr;
} Fb;

typedef struct {
    uint64_t pitch;
    uint32_t bpp;
} FbDevInfo;

int kb_fd;
Fb fb = {0};

void get_fb_info(int fd, uint64_t *pitchbuf, uint64_t *bppbuf) {
    FbDevInfo info = {0};
    if (read(fd, &info, sizeof(uint64_t) * 2) < 0) {
        printf("Failed to get framebuffer info.\n");
        exit(-1);
    }
    *pitchbuf = info.pitch, *bppbuf = info.bpp;
}

void DG_Init() {
    // Init framebuffer
    fb.fd = open("/dev/fb0", 0, 0);
    get_fb_info(fb.fd, &fb.pitch, &fb.bpp);
    if (fb.fd < 0) {
        printf("Failed to open framebuffer device.\n");
        exit(-1);
    }
    fb.ptr = mmap(NULL, 0, 0, 0, fb.fd, 0);
    printf("fb ptr = %p, pitch = %zu, bpp = %zu\n", fb.ptr, fb.pitch, fb.bpp);
    // Init keyboard
    kb_fd = open("/dev/kb0", 0, 0);
    if (kb_fd < 0) {
        printf("Failed to open keyboard device.\n");
        exit(-1);
    }
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

unsigned char to_doom_key(Key key) {
    switch (key) {
    case KeyEnter:      return KEY_ENTER;
    case KeyEscape:     return KEY_ESCAPE;
    case KeyLeftArrow:  return KEY_LEFTARROW;
    case KeyRightArrow: return KEY_RIGHTARROW;
    case KeyUpArrow:    return KEY_UPARROW;
    case KeyDownArrow:  return KEY_DOWNARROW;
    case KeyControl:    return KEY_FIRE;
    case KeySpace:      return KEY_USE;
    case KeyShift:      return KEY_RSHIFT;
    default:            return 'a' + key;
    }
}

int DG_GetKey(int* pressed, unsigned char* doomKey) {
    Key key;
    if (read(kb_fd, &key, 1) < 0) return 0;
    if (key == KeyNoPress) return 0;
    *pressed = !(key & 0x80);
    *doomKey = to_doom_key(key & 0x7f);
    return 1;
}

void DG_SetWindowTitle(const char * title) {}

int main(int argc, char **argv) {
    printf("\x1b[2J");
    for (size_t i = 0; i < 8; i++) printf("\n");
    doomgeneric_Create(argc, argv);
    for (;;) {
        doomgeneric_Tick();
    }
    close(fb.fd);
    close(kb_fd);
    return 0;
}
