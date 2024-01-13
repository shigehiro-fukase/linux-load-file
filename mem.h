// SPDX-License-Identifier: MIT
#ifndef ___MEM_H___
#define ___MEM_H___

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */

static inline unsigned long mmap_sys_pagesize(void) {
    unsigned long size = 0;
    long ret = sysconf(_SC_PAGE_SIZE);
    if (ret > 0) size = (unsigned long) ret;
    else size = 0x1000U; // error, default 4Kib
    return size;
}

#ifndef PROT_NOCACHE
#define PROT_NOCACHE 0
#endif // PROT_NOCACHE
#ifndef MAP_PHYS
#define MAP_PHYS 0
#endif // MAP_PHYS
#ifndef NOFD
#define NOFD -1
#endif // NOFD
static inline void * mmap_device_memory(const char * dev, void * addr, size_t len, uintptr_t physical, int * fd) {
    static int mmap_fd = NOFD;
    void * va;
    int prot = PROT_NOCACHE | PROT_READ | PROT_WRITE;
    int flags = 0;
    unsigned long page_size;
    uintptr_t map_pa, map_off, page_mask;
    size_t map_sz;

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
    page_size = mmap_sys_pagesize();
    page_mask = ((uintptr_t)page_size -1);
    map_pa = physical & ~page_mask;
    map_off = physical - map_pa;
    map_sz = (len + map_off + page_mask) & ~page_mask;

    va = mmap(addr, map_sz, prot,
            (flags & ~MAP_TYPE)
            // | MAP_ANONYMOUS
            | MAP_PHYS
            | MAP_SHARED
            , mmap_fd, map_pa);
    if (!va) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (fd) {
        *fd = mmap_fd;
    }
    return (void*)(((uint8_t*)va) + map_off);
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

    if ((fd = open(path, O_RDONLY)) < 0) {
        printf("%s: Cannot open file to read '%s'"NL, __progname, path);
        ret = -1;
        goto L_RETURN;
    }

    if ((va = mmap_device_memory(NULL, NULL, len, pa, &mem_fd)) == NULL) {
        printf("%s: Cannot map memory 0x%016lX (+0x%lX)"NL, __progname, pa, len);
        ret = -1;
        goto L_RETURN;
    }
    if ((uintptr_t)va == ~0ull) {
        printf("%s: Cannot map memory 0x%016lX (+0x%lX)"NL, __progname, pa, len);
        ret = -1;
        goto L_RETURN;
    }
    printf("%s: VA 0x%p"NL, __progname, va);

    n = read(fd, va, len);
    printf("%s: Load %ld of %lx bytes."NL, __progname, n, len);

    munmap_device_memory(mem_fd);

L_RETURN:
    if (fd >= 0) close(fd);
    return ret;
}

#endif /* ___MEM_H___ */

