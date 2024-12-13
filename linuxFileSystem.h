#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_FILES 100
#define MAX_NAME_LEN 256
#define MAX_CONTENT_LEN 4096

// Structure to store file information
struct file {
    char name[MAX_NAME_LEN];
    char content[MAX_CONTENT_LEN];
    size_t size;
};

static struct file files[MAX_FILES];
static int file_count = 0;

// Function to get attributes of files/directories
static int myfs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    // Root directory
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    // Check for files
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) { // Skip leading '/'
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = files[i].size;
            return 0;
        }
    }

    return -ENOENT; // File not found
}
// Function to list files in a directory
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struc>
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // List all files
    for (int i = 0; i < file_count; i++) {
        filler(buf, files[i].name, NULL, 0);
    }

    return 0;
}

// Function to read file contents
static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_in>
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            size_t len = files[i].size;
            if (offset >= len)
                return 0;

            if (offset + size > len)
                size = len - offset;

            memcpy(buf, files[i].content + offset, size);
            return size;
        }
    }

    return -ENOENT;
}

// Function to write to files
static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_>
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            if (offset + size > MAX_CONTENT_LEN)
                size = MAX_CONTENT_LEN - offset;
               memcpy(files[i].content + offset, buf, size);
            files[i].size = offset + size;
            return size;
        }
    }

    return -ENOENT;
}

// Function to create new files
static int myfs_mknod(const char *path, mode_t mode, dev_t rdev) {
    if (file_count >= MAX_FILES)
        return -ENOSPC;

    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0)
            return -EEXIST;
    }

    strncpy(files[file_count].name, path + 1, MAX_NAME_LEN - 1);
    files[file_count].size = 0;
    file_count++;

    return 0;
}

// Function to delete a file
static int myfs_unlink(const char *path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            for (int j = i; j < file_count - 1; j++) {
                files[j] = files[j + 1];
            }
            file_count--;
            return 0;
        }
    }

    return -ENOENT; // File not found
}

// Function to handle file timestamps
static int myfs_utimens(const char *path, const struct timespec tv[2]) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            // We are not storing timestamps, but accept the request
            return 0;
        }
    }
    return -ENOENT; // File not found
}

// Function to open a file
static int myfs_open(const char *path, struct fuse_file_info *fi) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            return 0; // File found, open successful
        }
    }
    return -ENOENT; // File not found
}

// Function to truncate or resize a file
static int myfs_truncate(const char *path, off_t size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(path + 1, files[i].name) == 0) {
            if (size > MAX_CONTENT_LEN)
                return -EFBIG; // File too large

            files[i].size = size;
            memset(files[i].content + size, 0, MAX_CONTENT_LEN - size); // Clear beyond new size
            return 0;
        }
    }
    return -ENOENT; // File not found
}

// FUSE operations structure
static struct fuse_operations myfs_operations = {
    .getattr = myfs_getattr,
    .readdir = myfs_readdir,
    .read = myfs_read,
    .write = myfs_write,
    .mknod = myfs_mknod,
    .unlink = myfs_unlink,
    .utimens = myfs_utimens,
    .open = myfs_open,
    .truncate = myfs_truncate,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_operations, NULL);
}
