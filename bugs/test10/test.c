/*
 * run: test10
 *
gfxconsole: rows 64, columns 170, extray 0

MAGENTA KERNEL PANIC

UPTIME: 4379ms
BUILDID GIT_E61158F484D61AB9816274041F7005DBDBCEC8B6_DIRTY_LOCAL

stopping other cpus
panic (caller 0xffffffff80194d7a frame 0xffffff9013e74e50): DEBUG ASSERT FAILED at (kernel/kernel/thread.c:541): t->magic == TH
READ_MAGIC
platform_halt suggested_action 0 reason 9
Halting...
bt#00: 0xffffffff8017823c
bt#01: 0xffffffff80129d49
bt#02: 0xffffffff80194d7a
bt#03: 0xffffffff80156264
bt#04: 0xffffffff80157de2
bt#05: 0xffffffff801576f3
bt#06: 0xffffffff80137894
bt#07: 0xffffffff8013daaf
bt#08: 0xffffffff801378c9
bt#09: 0xffffffff80113fe1
bt#10: end
Halted

current guess: use-after-free, the thread ref is not discarded after
it is killed the first time and its memory freed, and later the
assertion notices that its memory is corrupted.

I have other similar crashes that are likely variations on this one
(though often those crashes happen after the test process exits,
this one is triggered by the final system call).
 */
#include <stdio.h>
#include <stdlib.h>
#include <magenta/syscalls.h>
#include <magenta/process.h>

void xerr(const char *msg, mx_status_t st) {
    if(st != MX_OK) {
        printf("%s failed %d\n", msg, st);
        exit(1);
    }
}

int 
main(int argc, char **argv)
{
    mx_handle_t proc, thr, dummy;
    mx_status_t st;

    st = mx_process_create(mx_job_default(), "proc", 4, 0, &proc, &dummy);
    xerr("process_create", st);
    st = mx_thread_create(proc, "thread", 5, 0, &thr);
    xerr("thread_create", st);
    st = mx_task_kill(thr);
    xerr("task_kill", st);
    st = mx_task_kill(thr);
    xerr("task_kill", st);

    printf("done %d\n", st);
    return 0;
}
