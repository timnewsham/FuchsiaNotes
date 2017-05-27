# Filesystem Protocol

The filesystem is an illusion based on the cooperation of 
file servers and the mxio library.  When a process starts up
in the normal fashion, it receives a set of open handles.
Among these handles is a root filesystem handle, which is a
channel to a filesystem server.  The program accesses files
by using the mxio library to send messages to fileservers.
This can result in new handles being opened to new fileservers.
These calls are usually hidden from the programmer behind
standard POSIX calls such as "open", "read", and "write".

Files are currently accessed by sending mxrio_msg_t messages.
This is subject to change, and the protocol definitions will
likely be replaced with IDL definitions in the future.  for the
time being, the structure is:

```c
struct mxrio_msg {
    mx_txid_t txid;                    // transaction id
    uint32_t op;                       // opcode
    uint32_t datalen;                  // size of data[]
    int32_t arg;                       // tx: argument, rx: return value
    union {
        int64_t off;                   // tx/rx: offset where needed
        uint32_t mode;                 // tx: Open
        uint32_t protocol;             // rx: Open
        uint32_t op;                   // tx: Ioctl
    } arg2;
    int32_t reserved;
    uint32_t hcount;                   // number of valid handles
    mx_handle_t handle[4];             // up to 3 handles + reply channel handle
    uint8_t data[MXIO_CHUNK_SIZE];     // payload
};
```

Most operations are performed by filling out a message structure
with the desired operation, arguments and data, sending it off
to a server, and receiving back results in the same format.
However, two operations, MXRIO_OPEN and MXRIO_CLONE, require opening 
a new channel pair, passing in one half as part of the request message,
and then receiving an mxrio_object_t response on the other half
of the message pair.  The response may also include additional
handles.  This is needed for handling sockets, for example, which
uses a separate kernel socket object for IO.

```c
typedef struct {
    // Required Header
    mx_status_t status;
    uint32_t type;

    // Optional Extra Data
    uint8_t extra[MXRIO_OBJECT_EXTRA];
    
    // OOB Data
    uint32_t esize;
    uint32_t hcount;
    mx_handle_t handle[MXIO_MAX_HANDLES]; 
} mxrio_object_t;
```

This procedure is used whenever a new channel is needed. It allows
the file server to
forward the request on to another fileserver as necessary.  Many
of these responses will only have the "status" field, which gives
an error response if necessary, and the "type" field, which says
which kind of connection is used.

## Example

The following example illustrates these principles.  It interacts
with the root file server to open a file (on a second channel),
and read data from it. (testRio/testRio.c)

## Operations

Here are some common message types used in the filesystem protocol:

- MXRIO_OPEN - open(filename=msg.data, flags=msg.arg, mode=msg.arg2.mode)
    - handles in: objhand
    - handles out: XXX?
    - result: mxrio_object_t sent on objhand
    - on success, objhand references the opened file
- MXRIO_CLONE - clone(flag=msg.arg)
    - handles in: objhand
    - handles out: XXX?
    - result: mxrio_object_t sent on objhand if MXRIO_OFLAG_PIPELINE flag not
      set (otherwise nothing?)
    - on success, duplicate server state, and process messages on objhand,
      keeping the original handle open as well
- MXRIO_CLOSE - close()
    - result: msg? XXX confirm this
    - on success: the file is closed and the peer channel handle is closed
- MXRIO_READ - read(size=msg.arg, offset=serverState.offset)
    - result: mxrio_msg_t with data in msg.data[0..msg.datalen]
    - on success, data from the file at the current offset is returned, 
      and the server updates its offset associated with this channel.
- MXRIO_READ_AT - read(size=arg, offset=msg.arg2.off)
    - result: mxrio_msg_t with data in msg.data[0..msg.datalen]
    - on success, data from the file at the given offset is returned.
- MXRIO_WRITE - write(data=msg.data, datalen=msg.datalen, off=serverState.offset)
    - result: mxrio_msg_t
    - on success, data from msg.data[0..msg.datalen] is written into
      the file associated with the channel at the current offset (or
      the end of the file, if it was opened with O_APPEND).  The 
      server updates its offset associated iwth the channel.
- MXRIO_WRITE_AT - write(data=msg.data, datalen=msg.datalen, off=msg.arg2.off)
    - result: mxrio_msg_t
    - on success, data from msg.data[0..msg.datalen] is written into
      the file associated with the channel at the specified offset.
- MXRIO_SEEK
- MXRIO_STAT
- MXRIO_SETATTR
- MXRIO_READDIR
- MXRIO_IOCTL_1H - 1 handle
- MXRIO_IOCTL
- MXRIO_TRUNCATE
- MXRIO_RENAME - 1 handle
- MXRIO_LINK - 1 handle
- MXRIO_MMAP
- MXRIO_SYNC
- MXRIO_UNLINK
- MXRIO_GETSOCKNAME
- MXRIO_GETPEERNAME
- MXRIO_GETSOCKOPT
- MXRIO_SETSOCKOPT
- MXRIO_GETADDRINFO


## Connection Types

Not all file servers speak exactly the same protocol.  To account
for this, cloning and opening a new handle returns a type field specifying
which flavor the server supports.  Currently these are:

- MXIO_PROTOCOL_REMOTE  - "normal" remote file server object.  References
        a file that can be read, written, seeked, ioctl, etc.
- MXIO_PROTOCOL_PIPE    - communication channels that dont support seeking,
        or opening new files
- MXIO_PROTOCOL_VMOFILE - references to kernel VMO objects, not clonable, 
        waitable, no ioctl, reads and writes to straight to the kernel.
- MXIO_PROTOCOL_SOCKET  - network socket objects.  Supports reading and
        writing, sendmsg, recvmsg, clone, waiting. Does not support seek
        or clone.  These objects are referenced by two handles.  The
        first handle is the normal filesystem channel.  The second is
        a kernel socket object which is used for IO.  It is capable of
        transmitting individual bytes in a stream.
- MXIO_PROTOCOL_SERVICE - References an object that doesnt speak the
        filesytem protocol except to support opening handles.  This
        provides a mechanism to get a handle through filesystem operations
        and then use it for other purposes.

----
# TODO
- explore ".." handling.. it is odd
  for example /tmp/.. is valid, but /tmp/../tmp is not.
- some pointers to system/ulib/mxio/remoteio.c
- some pointers to devmgr, tmpfs, vfs, etc
- note: mmap gives full region access since you cant make sub-regions

