/*
 * run: test8
 *
MAGENTA KERNEL PANIC

UPTIME: 4258ms
BUILDID GIT_B6FB4AB5BEAA51B099A5099300154A023E7AD0E6_DIRTY_LOCAL

stopping other cpus
panic (caller 0xffffffff8016183d frame 0xffffff9013e74d90): DEBUG ASSERT FAILED at (kernel/kernel/vm/vm_mapping.cpp:96): size !
= 0 && IS_PAGE_ALIGNED(base) && IS_PAGE_ALIGNED(size)
platform_halt suggested_action 0 reason 9
Halting...
bt#00: 0xffffffff80182c3c
bt#01: 0xffffffff8016bf79
bt#02: 0xffffffff8016183d
bt#03: 0xffffffff801a1ac8
bt#04: 0xffffffff8015599f
bt#05: 0xffffffff801354b4
bt#06: 0xffffffff8013bd34
bt#07: 0xffffffff80135516
bt#08: 0xffffffff8011432d
bt#09: end
Halted

This seems to happen when the size 0xffffffffffffff00 is rounded 
up to zero in VmAddressRegion::Protect:

    size = ROUNDUP(size, PAGE_SIZE);

triggering an assertion in VmMapping::ProtectLocked that size is nonzero:

    DEBUG_ASSERT(size != 0 && IS_PAGE_ALIGNED(base) && IS_PAGE_ALIGNED(size));

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
        printf("vmo create failed!\n");
        return 1;
    }
    st = mx_vmar_map(mx_vmar_root_self(), 0, h, 0, 0x48000ff0000, 0x23, malloc(0x80008d00));
    if(st != NO_ERROR) {
        printf("vmar map fliaed!\n");
        return 1;
    }
    st = mx_vmar_protect(mx_vmar_root_self(), 0x1b00000000, 0xffffffffffffff00, 1);
    printf("done %d\n", st);
    return 0;
}
