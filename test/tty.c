#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

uv_loop_t *loop;
uv_tty_t tty;

void on_tty_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
void on_tty_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void on_tty_write(uv_write_t *req, int status);

/**
 * Buffer we use to read.
 */
char buffer[0xffff];

int main() {
    printf("== %s\n", __func__);
    loop = uv_default_loop();

    // Open a TTY for reading and writing
    uv_tty_init(loop, &tty, 0, 1); // 0 for stdin, 1 for stdout

    // Set TTY to raw mode to capture individual key presses
    uv_tty_set_mode(&tty, UV_TTY_MODE_RAW);

    // Start reading from TTY
    uv_read_start((uv_stream_t*)&tty, on_tty_alloc, on_tty_read);

    // Start the event loop
    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}

void on_tty_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    // Allocate buffer for reading TTY input
    printf("== %s suggested_size: %zu %zu\n", __func__, suggested_size, buf->len);
    buf->base = buffer;
    buf->len = 0xffff;
}

void on_tty_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    printf("== %s\n", __func__);
    if (nread > 0) {
        // Display pressed keys
        for (ssize_t i = 0; i < nread; ++i) {
            printf("Key pressed: %c (%u)\n", buf->base[i], buf->base[i]);
            if (buf->base[i] == 27) {
                uv_stop(loop);
            }
        }
    } else if (nread < 0) {
        fprintf(stderr, "Error reading from TTY: %s\n", uv_strerror(nread));
    }

    // Free the buffer
    //free(buf->base);
}

void on_tty_write(uv_write_t *req, int status) {
    printf("== %s\n", __func__);
    if (status < 0) {
        fprintf(stderr, "Error writing to TTY: %s\n", uv_strerror(status));
    }

    // Clean up the write request
    uv_close((uv_handle_t*)req->handle, NULL);
}
