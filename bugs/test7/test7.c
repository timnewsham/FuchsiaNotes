/*
 * run: test7
 *
MAGENTA KERNEL PANIC

UPTIME: 7744ms
BUILDID GIT_B6FB4AB5BEAA51B099A5099300154A023E7AD0E6_DIRTY_LOCAL

stopping other cpus
panic (caller 0xffffffff8014b3d0 frame 0xffffff9013e60ee0): DEBUG ASSERT FAILED at (kernel/kernel/vm/vm_mapping.cpp:385): IS_PAGE_ALIGNED(len)
platform_halt suggested_action 0 reason 9
Halting...
bt#00: 0xffffffff8016aa80
bt#01: 0xffffffff8015c88e
bt#02: 0xffffffff8014b3d0
bt#03: 0xffffffff80134f38
bt#04: 0xffffffff80114307
bt#05: end
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
    mx_handle_t h;
    mx_status_t st = mx_vmo_create(8192, 0, &h);
    if(st != NO_ERROR) {
        printf("vmo create failed %d\n", st);
        return 1;
    }
    st = mx_vmar_map(mx_vmar_root_self(), 0, h, 0x2010000040000, 0x10000050, 0x411, 0);
    printf("done %d\n", st);
    return 0;
}
