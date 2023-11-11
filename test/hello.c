#include <stdio.h>
#include <uv.h>

int64_t counter = 0;

void idle_cb(uv_idle_t *handle) {
    printf("Idle callback\n");
    counter++;

    if (counter >= 5) {
        uv_stop(uv_default_loop());
        printf("uv_stop() called\n");
    }
}

void prep_cb(uv_prepare_t *handle) {
    printf("Prep callback\n");
}

int main() {
    uv_idle_t idler;
    uv_prepare_t prep;

    uv_idle_init(uv_default_loop(), &idler);
    uv_idle_start(&idler, idle_cb);

    uv_prepare_init(uv_default_loop(), &prep);
    uv_prepare_start(&prep, prep_cb);

    uv_passwd_t pwd;
    size_t len;
    int r;

    r = uv_os_get_passwd(&pwd);
    if (r == 0) {
        printf(
            "User:  %s\n"
            "Home:  %s\n"
            "Shell: %s\n",
            pwd.username,
            pwd.homedir,
            pwd.shell
        );
    }


    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    return 0;
}