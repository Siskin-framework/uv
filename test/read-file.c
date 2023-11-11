#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

uv_loop_t *loop;
uv_fs_t file_req;
uv_buf_t iov;

void on_file_read(uv_fs_t *req);

int main() {
    loop = uv_default_loop();

    const char *filename = "../uv.nest";

    // Open the file
    uv_fs_open(loop, &file_req, filename, O_RDONLY, 0, NULL);

    // Check if the file was opened successfully
    if (file_req.result < 0) {
        fprintf(stderr, "Error opening file: %s\n", uv_strerror(file_req.result));
        return 1;
    }

    // Allocate buffer for reading file content based on the file size
    uv_fs_fstat(loop, &file_req, file_req.result, NULL);
    size_t file_size = file_req.statbuf.st_size;
    printf("File size: %llu\n", file_size);
    iov = uv_buf_init((char*)malloc(file_size), file_size);

    // Read file asynchronously
    uv_fs_read(loop, &file_req, file_req.result, &iov, 1, 0, on_file_read);

    // Start the event loop
    uv_run(loop, UV_RUN_DEFAULT);

    // Cleanup
    uv_fs_req_cleanup(&file_req);
    free(iov.base);

    return 0;
}

void on_file_read(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "Error reading file: %s\n", uv_strerror(req->result));
    } else if (req->result > 0) {
        // File content is available in iov.base
        printf("Read %ld bytes:\n%s\n", req->result, iov.base);

        // Continue reading the file if needed
        uv_fs_read(loop, &file_req, file_req.result, &iov, 1, -1, on_file_read);
    } else {
        // File read is complete
        printf("File read complete.\n");

        // Close the file
        uv_fs_close(loop, &file_req, file_req.result, NULL);
    }
}
