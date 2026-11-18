#include <kisaki/printk.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <deps/stb_sprintf.h>

putc_sink kernel_putc;

void set_putc_sink(putc_sink new_putc) {
    kernel_putc = new_putc;
}

char* stb_callback(char const * buf, void * user, int len) {
    (void)user;

    for (int i = 0; i < len; i++) {
        if (kernel_putc) kernel_putc(buf[i]);
    }
    return (char *)buf;
}

void printk(const char* fmt, ...) {
    char buf[STB_SPRINTF_MIN];

    va_list args;
    va_start(args, fmt);

    stbsp_vsprintfcb(stb_callback, NULL, buf, fmt, args);

    va_end(args);
}
