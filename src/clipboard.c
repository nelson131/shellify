#include "clipboard.h"

char* clipboard_get() {
    if (getenv("WAYLAND_DISPLAY")) return clipboard_read("wl-paste");
    if (getenv("DISPLAY"))
        return clipboard_read("xclip -selection clipboard -o");

    errlog(ERR_CLIPBOARD_FAILED, "clipboard_get:failed");
    return NULL;
}

char* clipboard_read(const char* cmd) {
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        errlog(ERR_FILE_OPENING, "clipboard_read:fp");
        return NULL;
    }

#define BUF_BASE_SIZE 512
    char*  buf = malloc(BUF_BASE_SIZE);
    size_t size = 0;
    size_t cap = BUF_BASE_SIZE;
    if (!buf) {
        pclose(fp);
        errlog(ERR_MALLOC_NULL, "clipboard_read:buf");
        return NULL;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (size + 1 >= cap) {
            size_t new_cap = cap * 2;
            char*  temp = realloc(buf, new_cap);
            if (!temp) {
                errlog(ERR_MALLOC_NULL, "clipboard_read:temp");
                pclose(fp);
                free(buf);
                return NULL;
            }

            cap = new_cap;
            buf = temp;
        }
        buf[size++] = c;
    }

    buf[size] = '\0';
    return buf;
}
