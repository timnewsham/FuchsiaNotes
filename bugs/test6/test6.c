/*
 * run: test6
gfxconsole: rows 64, columns 170, extray 0

MAGENTA KERNEL PANIC

UPTIME: 4738ms
BUILDID GIT_867CFF6B1131374F17446A032A4879117C098D5A_DIRTY_LOCAL

stopping other cpus
panic (caller 0xffffffff8014b41e frame 0xffffff9013e7cf20): DEBUG ASSERT FAILED at (kernel/kernel/vm/vm_object_paged.cpp:516): 
end > start
platform_halt suggested_action 0 reason 9
Halting...
bt#00: 0xffffffff80169b5c
bt#01: 0xffffffff8015f1b0
bt#02: 0xffffffff8014b41e
bt#03: 0xffffffff8013448d
bt#04: 0xffffffff80113e40
bt#05: end
Halted
entering panic shell loop

In VmObjectPaged::DecommitRange(), offset comes from syscall arg:
    uint64_t new_len;
    if (!TrimRange(offset, len, size_, &new_len))
        return ERR_OUT_OF_RANGE;

    // figure the starting and ending page offset
    uint64_t start = PAGE_ALIGN(offset);
    uint64_t end = ROUNDUP_PAGE_SIZE(offset + new_len);
    DEBUG_ASSERT(end > offset);
    DEBUG_ASSERT(end > start);

and if offset is 0x10 then start and end are 0x1000, and "end > start" fails.
 */
#include <stdio.h>
#include <stdlib.h>
#include <magenta/syscalls.h>
#include <magenta/process.h>

int 
main(int argc, char **argv)
{
    mx_handle_t h;
    mx_status_t st = mx_vmo_create(8192, 0, &h);
    if(st != MX_OK) {
        printf("vmo create failed %d\n", st);
        return 1;
    }
    uint64_t off = 0x10;
    uint64_t sz = 0x100;
    size_t bufsz = 0x100;
    void *buf = malloc(bufsz);
    st = mx_vmo_op_range(h, MX_VMO_OP_DECOMMIT, off, sz, buf, bufsz);
    printf("done %d\n", st);
    return 0;
}
