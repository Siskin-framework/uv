#include <uv.h>
#include <stdio.h>
#include <assert.h>

/**
 * Reference to our event loop.
 */
uv_loop_t * loop;

/**
 * Work requests for fs actions.
 */
uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;
uv_fs_t close_req;

/**
 * Buffer we use to read.
 */
char buffer[0xffff];

uv_buf_t iov;

/**
 * Function heads.
 */
void open_cb(uv_fs_t * req);
void read_cb(uv_fs_t * req);
void write_cb(uv_fs_t * req);
void close_cb(uv_fs_t * req);

/**
 * Execution entrance point.
 * Gets passed arguments from console in argv array.
 */
int main(int argc, const char ** argv) {
    /* request our event loop */
    loop = uv_default_loop();

    /* return if user did not pass any filename */
    if (!argv[1]) {
        printf("Please pass a filename as argument.\n");

        return 1;
    }

    printf("opening file: %s\n", argv[1]);
    /* open file as user in read-only mode */
    uv_fs_open(loop, &open_req, argv[1], O_RDONLY, 0, open_cb);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
}

/**
 * Callback executed when file is opened.
 */
void open_cb(uv_fs_t * req) {
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    assert(req == &open_req);
    if (req->result >= 0) {
        iov = uv_buf_init(buffer, sizeof(buffer));
        uv_fs_read(uv_default_loop(), &read_req, req->result,
                   &iov, 1, -1, read_cb);
    }
    else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
    }
}

/**
 * Callback executed when read file.
 */
void read_cb(uv_fs_t * req) {
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    }
    else if (req->result == 0) {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    }
    else if (req->result > 0) {
        iov.len = req->result;
        uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, write_cb);
    }
}

void write_cb(uv_fs_t * req) {
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
    }
    else {
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, read_cb);
    }
}

/**
 * Callback executed when file was closed.
 */
void close_cb(uv_fs_t * req) {
    int result = req->result;

    if (result < 0) {
        fprintf(stderr, "Error on closing file: %s\n.", uv_strerror(result));
    }

    uv_fs_req_cleanup(req);

    /* output read data to console */
    printf("%s\n", buffer);
}