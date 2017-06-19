# Overview

Magenta is the base operating system, including the kernel and
device drivers and support infrastructure.

Magenta is written in a subset of modern C++ [https://github.com/fuchsia-mirror/magenta/blob/master/docs/cxx.md], 
with C (25%) and asm (3%).
It is based on a microkernel, which supports capabilities, 
message passing and limited device support (mostly for supporting
low-level bus operations).  Device drivers and
filesystems are written as processes using a DDK interface.

# Infrastructure

* stuff for loading drivers
* filesystem to contain drivers
* library to access syscalls
* library support for misc stuff like executing procs
* "misc" drivers for providing environment support, like "root handle"

# Security

* there are no users. only capabilities.
* capabilities, based on kernel object handles and rights.  Each
  handle is associated with rights that say how the handle may be used.
* object handles are owned by a single process and their handles are not
  valid in any other processes.  
* multiple object handles may reference the same object
* object handles can be transfered to other processes via message passing.
  while in transit they are not owned by the sender or the receiver, but
  are owned by the kernel.
* kernel has policy settings for limiting operations. XXX investigate more XXX QueryPolicy
* handles have weak obfuscation.  each process has a random mask
  that handles are xor'd against.

# Drivers and DDK

# Kernel Code

* Handle lib/magenta/include/magenta/handle.h record the owning process,
  the object (dispatcher) they reference, and the rights associated with
  the handle.
* Dispatcher lib/magenta/include/magenta/dispatcher.h is the base class
  for kernel objects.
* JobDispatcher ./lib/magenta/include/magenta/job_dispatcher.h is an
  object representing a job.  Jobs group together processes and other jobs.
* ProcessDispatcher ./lib/magenta/include/magenta/process_dispatcher.h
  are objects for managing processes.  It contains a list of UserThreads,
  a reference to a VmAspace, a list of Handles.
  and a reference to a
* ThreadDispatcher ./lib/magenta/include/magenta/thread_dispatcher.h
  represents a thread by referencing a 
  UserThread ./lib/magenta/include/magenta/user_thread.h.
* rights are defined in system/public/magenta/types.h
* signals are defined in system/public/magenta/types.h
* important types: 
    * mx_koid_t are object IDs

# Random Tips

* see [https://github.com/fuchsia-mirror/magenta/blob/master/docs/hacking.md]
  for info on getting to the shell window, and using keys.  Alt-Esc to
  switch to terminals, Alt-Fn to jump to terminals, Alt-tab to switch
  to next terminal.
* The shell has a few interesting builtins for hacking:
    * `k help` and `k mx help.  Sends messages to a kernel
      debugging interface.  Output goes to serial console.
      See magenta/third_party/uapp/dash/src/bltin/magenta.c mxc_k() for details
      See kernel/lib/magenta/diagnostics.cpp cmd_diagnostics() for mx commands.
    * from shell run `@?` and `@ bootstrap launch spinning_square`.
      Sends a message to dmctl, which sends it to the application manager
      which tries to launch an application.
      See magenta/third_party/uapp/dash/src/bltin/magenta.c mxc_at() for details
      See application/src/manager/command_listener.cc
    * from shell run `dm help`, `dm reboot`, `dm "kerneldebug help` (same as `k help`), `dm @?` (same as `@?`)
      Sends a message directly to dmctl
      XXX show equivalent command as launching bootstrap?
      See magenta/third_party/uapp/dash/src/bltin/magenta.c mxc_dm() for details
      See system/core/devmgr/dmctl.c, related devmgr files for more, and /dev/class/misc/dmctl for more

