/*
 * run: test5 27009c070000 30081000000
 * a few times and get a panic:
gfxconsole: rows 64, columns 170, extray 0

MAGENTA KERNEL PANIC

UPTIME: 19447ms
BUILDID GIT_867CFF6B1131374F17446A032A4879117C098D5A_DIRTY_LOCAL

stopping other cpus
panic (caller 0xffffffff80157b17 frame 0xffffff9013e68eb0): DEBUG ASSERT FAILED at (kernel/kernel/vm/vm_mapping.cpp:239): base 
>= base_ && base - base_ < size_
platform_halt suggested_action 0 reason 9
Halting...
bt#00: 0xffffffff80169b5c
bt#01: 0xffffffff8015d427
bt#02: 0xffffffff80157b17
bt#03: 0xffffffff8015897a
bt#04: 0xffffffff8014a4b2
bt#05: 0xffffffff80134aa5
bt#06: 0xffffffff80113eb4
bt#07: end
Halted
entering panic shell loop
 */
#include <stdio.h>
#include <stdlib.h>
#include <magenta/syscalls.h>
#include <magenta/process.h>

int 
main(int argc, char **argv)
{
    char *endp;
    uintptr_t base = 0x1000;
    size_t sz = 0x1000;
    if(argc > 1)
        base = strtoul(argv[1], &endp, 16);
    if(argc > 2)
        sz = strtoul(argv[2], &endp, 16);
    printf("%lx %lx\n", base, sz);
    mx_status_t st = mx_vmar_unmap(mx_vmar_root_self(), base, sz);
    printf("done %d\n", st);
    return 0;
}
