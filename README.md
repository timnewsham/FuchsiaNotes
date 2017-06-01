# Google's Magenta / Fuchsia

This project is designed to share some notes on Google's Magenta / Fuchsia 
Operating system.

Roughly speaking, Magenta is the kernel and core operating system services,
and Fuchsia is the larger ecosystem built on top of Magenta.  Magenta's
microkernel provides limited support for device busses, for processes
and threads, and for IPC.  Servers and libraries build on this to provide
traditional operating system services such as device drivers, files, 
filesystems, network sockets, and shared libraries. This layer also
provides some of the niceties expected in a modern operating system, such
as shell and utilities.  Fuchsia builds on this base to provide
(stuff about ephemeral code from the network, sandboxing, post-API,
language neutral, graphical environment, etc).

Magenta and Fuchsia are in development and evolving rapidly.  These
notes are my attempt at documenting a moving target.
Its possible some details are wrong and very likely that they will
become wrong. 

# Notes

- [Magenta Overview](Magenta.md)
    - [Syscalls](Syscalls.md)
    - [DevMgr](DevMgr.md)
    - [FileSystem](FileSystem.md)
- [BuildNotes](BuildNotes.md)

# References

- [General Fuchsia docs](https://github.com/fuchsia-mirror/docs)
- [Magenta specific docs](https://github.com/fuchsia-mirror/magenta/tree/master/docs)

