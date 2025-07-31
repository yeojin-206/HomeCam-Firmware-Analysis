" hook.c "

#include <dlfcn.h>
#include <string.h>
#include <stdio.h> 

typedef int (*orig_open_f_type)(const char *pathname, int flags, ...);

int open(const char *pathname, int flags, ...) {
    orig_open_f_type orig_open = (orig_open_f_type)dlsym(RTLD_NEXT, "open");

    if (strcmp(pathname, "/dev/motor") == 0) {
        printf("[HOOK] Intercepted open(\"/dev/motor\"). Returning fake success.\n");
        return 99;
    }

    return orig_open(pathname, flags);
}
int reboot(int op) {
    printf("[HOOK] Reboot call blocked! (op: 0x%x)\n", op);
    return 0;
}
