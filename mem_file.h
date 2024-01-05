// SPCX-License-Identifier: MIT
#ifndef ___MEM_H___
#define ___MEM_H___

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#ifndef PROT_NOCACHE
#define PROT_NOCACHE 0
#endif // PROT_NOCACHE
#ifndef MAP_PHYS
#define MAP_PHYS 0
#endif // MAP_PHYS
#ifndef NOFD
#define NOFD -1
#endif // NOFD
static inline void * mmap_device_memory(const char * dev, void * addr, size_t len, uint64_t physical, int * fd) {
    static int mmap_fd = NOFD;
    void * va;
    int prot = PROT_NOCACHE | PROT_READ | PROT_WRITE;
    int flags = 0;
    if (!dev) {
        dev = "/dev/mem";
    }
    if (mmap_fd < 0) {
        mmap_fd = open(dev, O_RDWR | O_SYNC);
        if (mmap_fd < 0) {
            perror("open /dev/mem");
            exit(EXIT_FAILURE);
        }
    }
    va = mmap(addr, len, prot,
            (flags & ~MAP_TYPE)
            // | MAP_ANONYMOUS
            | MAP_PHYS
            | MAP_SHARED
            , mmap_fd, physical);
    if (!va) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (fd) {
        *fd = mmap_fd;
    }
    return va;
}
static inline void munmap_device_memory(int fd) {
    if (fd >= 0) {
        close(fd);
        fd = NOFD;
    }
}

#ifndef NL
#define NL "\n"
#endif

#if 0
#include <stdio.h>
static inline fpos_t file_get_size(FILE * fp) {
    fpos_t pos, size;
    if (!fp) return ((fpos_t)-1);
    fgetpos(fp, &pos); // current pos
    fseek(fp, 0, SEEK_END);
    fgetpos(fp, &size);
    fseek(fp, pos, SEEK_SET); // rewind to pos
    return size;
}
#endif

static inline int mem_ldfile(intptr_t pa, size_t len, const char * path) {
    int ret = 0;
    int fd = -1;
    int mem_fd = -1;
    void * va;
    ssize_t n;
    extern const char * __progname;
    size_t size = ((len + 0xFFFu) & ~0xFFFu);

    if ((fd = open(path, O_RDONLY)) < 0) {
        printf("%s: Cannot open file to read '%s'"NL, __progname, path);
        ret = -1;
        goto L_RETURN;
    }

    // if ((va = mmap_device_memory(NULL, NULL, len, pa, &mem_fd)) == NULL) {
    if ((va = mmap_device_memory(NULL, NULL, size, pa, &mem_fd)) == NULL) {
        printf("%s: Cannot map memory 0x%016lX (+0x%lX)"NL, __progname, pa, len);
        ret = -1;
        goto L_RETURN;
    }
    if ((uintptr_t)va == ~0ull) {
        printf("%s: Cannot map memory 0x%016lX (+0x%lX)"NL, __progname, pa, size);
        ret = -1;
        goto L_RETURN;
    }
    printf("%s: VA 0x%p"NL, __progname, va);

    // {
    // char buf[0x1000];
    // n = read(fd, buf, len);
    // printf("%s: Load %ld of %lx bytes."NL, __progname, n, size);
    // lseek(fd, 0, SEEK_SET);
    // }
    n = read(fd, va, len);
    printf("%s: Load %ld of %lx bytes."NL, __progname, n, size);

    munmap_device_memory(mem_fd);

L_RETURN:
    if (fd >= 0) close(fd);
    return ret;
}

#endif /* ___MEM_H___ */

