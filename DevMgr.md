------
# DEVMGR

This process handles userland startup, including starting up
device drivers and initializing the filesystem.  It manages
device enumeration and loads up drivers. (TODO more details).
It also constructs the root filesystem and the /dev filesystem,
populating devices into /dev as they become available.  It
plays an important role in system startup, launching several
important processes, such as console and virtual-console shells.
It notices new filesystems as they become available and mounts
some of them automatically.

## Environment Variables
DevMgr receives environment variables that are passed on the
kernel command line.  It responds to the following settings:
- `crashlogger.disable` - if set, do not run `crashlogger`
- `crashlogger.pt` - if set to `true`, pass `-pton` to `crashlogger` to enable support for Intel PT
- `devmgr.verbose` - enable verbose logging
- `ldso.trace` - pass the environment variable `LD_TRACE=1` to all new processes, to enable tracing of shared library loading
- `magenta.autorun.boot` - specifies a program to run once the system is running
- `magenta.autorun.system` - specifies a program to run once `/system` is mounted
- `magenta.nodename` - node name to pass to `netsvc`
- `netsvc.disable` - if set, do not run `netsvc`
- `startup.keep-log.visible` - if set, keep logging to kernel instead of to virtual console
- `TERM` - terminal type, passed in environment when starting the shell
- `virtcon.disable` - if set, do not launch shells on virtual consoles as they become available


## Logical flow

- At startup
    - initialize /dev
        - populates /dev/class 
        - create "/dev" device, with a channel that dispatches to dc_rio_handler
        - signal the "/dev" device with MX_USER_SIGNAL_0
        - mount "/dev" device
            - XXX ./system/core/devmgr/vfs-memory.cpp devfs_mount investigate
    - initialize /
        - setup bootfs
            - get startup handle for bootfs
            - ...
        - initialize vfs layer 
            - makes memfs for /, and addds /data, /volume, /dev/socket
            - makes channel for rootfs, dispatching with vn->Serve (causes vfs_handler to get invoked for messages)
            - sets root handle in mxio (libc), now libc calls can work
    - creates new job for services
    - optionally launches crashlogger
    - starts new thread which launches console shell
    - run /system/bin/init, with handles for applauncher and service requests
    - start "service-starter" thread
        - launch /boot/bin/netsvc, and /boot/bin/sh /boot/autorun, 
        - watch /dev/class/block and for each new file added
            - determine disk format
            - bind device to /boot/driver/gpt.so, /boot/driver/mbr.so, or automount /bobstore, minfs, or fat (/volume/efi-XX, /volume/fat-XX).
    - start "virtcon-starter" thread
        - watch "/dev/class/console" directory and for each new file added
            - if name starts with "vc", set it as active and launch shell
    - run the coordinator
- coordinator
    - initializes acpi and pcie
    - initialize /dev/misc, /dev/socket, /dev/platform
    - enumerate drivers
        - find drivers for /boot/driver, /boot/driver/test, /system/driver, /boot/lib/driver, /system/lib/driver
    - attempt to bind drivers to root, misc or platform device
    - forever dispatch work
        - if list_pending_work, remove an item and process_work on it
            - WORK_DEVICE_ADDED dispatch to dc_handle_new_device
                - search a list of all drivers to see if the new device can bind to one of them
        - otherwise dispatch the dc_port, which calls callbacks on events
- XXX tie in dc_handle_device
- dc_handle_device handles IO on the coordinator channel
    - on closed - remove device
    - on read call dc_handle_device_read and on failure remove device
        - dc_handle_device_read processes incoming messages to the coordinator
        - reads a dc_msg_t and two handles
        - unpacks data, name and args from msg
        - closes all handles unless DC_OP_ADD_DEVICE
        - DC_OP_ADD_DEVICE
        - DC_OP_REMOVE_DEVICE
        - DC_OP_BIND_DEVICE
        - DC_OP_DM_COMMAND - calls handle_dmctl_write
            - dump
            - help
            - reboot
            - poweroff
            - shutdown
            - ktraceon
            - ktraceoff
            - kerneldebug
            - @<cmd>
        - DC_OP_STATUS
- devfs_publish is used to put a device into /dev
    - make a fs node
    - if appropriate, link it into /dev/class/<proto>/
    - add it to the parent dir
    - notify anyone listening for changes

- TODO list
    - dm/dmctl
    - more details on vfs and how libc and devmgr cooperate to present filesystems
    - more stuff on mounting?
    - more stuff on driver loading and shared libs perhaps lots of details on matching and binding drivers?  perhaps this could go in separate ddk documentation 


------
# DEVHOST

This process hosts device drivers.  It supports a small number of RPC
calls for interacting with the device.  These are valid on the first
handle it inherits from devmgr (the parent), and on all the channels
opened to new devices in this devhost:

- MXRIO_OPEN
- DC_OP_CREATE_DEVICE_STUB - create new device,
- DC_OP_CREATE_DEVICE
- DC_OP_BIND_DRIVER

##Logical flow:

- main gets bootstrap handle from devmgr, and listens on it as the root_ios.
- root_ios dispatches the startup handle to dh_handle_dc_rpc
    - on closed, die
    - on readable call dh_handle_rpc_read, die on errors

- dh_handle_rpc_read
    - reads dc_msg_t and 3 handles, (ignoring errors?)
        - if msg op is MXRIO_OPEN, treat it as mxrio_msg_t(!) at dh_handle_open XXX more?
        - unpack msg into data,name,args, and handle:
        - DC_OP_CREATE_DEVICE_STUB
            - gets one or two handles - rpc, resource
            - makes a new shadow device 
            - dispatches to dh_handle_dc_rpc when handle readable/closed
        - DC_OP_CREATE_DEVICE
            - gets two or three handles - rpc, vmo, resource
            - finds driver
            - calls create op, if it exists
            - dispatches to dh_handle_dc_rpc when handle readable/closed
        - DC_OP_BIND_DRIVER
            - gets one handle - vmo
            - finds driver
            - calls bind op if it exists
            - sends response back
- dh_handle_dc_rpc dispatched when device handles readable/closed
    - on closed, die
    - on readable, calls dh_handle_rpc_read and dies on error

- XXX TODO
    - multiple related drivers can coexist in a devhost, like the framebuffer driver and the gfx console driver.  XXX figure out how these bind together


    - Devhost uses dh_port to wait for events
        - to watch for events on handles, a devhost_iostate_t is filled out with the handle and handler function, and port_watch is used to direct events to dh_port

------------
# comments
- conflating dc_msg_t and mxrio_msg_t is a bad idea.  
  this could be handled more gracefully.


------
# random
- vfs layer uses system/ulib/fs/*.cpp
    - MxioDispatcher::Create
        - mxio_dispatcher_create makes a port, a list and sets cb
        - sets callback to mxrio_handler
        - calls mxio_dispatcher_start to create new thread mxio_dispatcher_thread which watches on the port for readable (call cb) or closable, and closes the port on close
 mxrio_handler gets an arg which is another cb.
    MxioDispatcher::AddVFSHandler sets the "func" which becomes this cb.

- mxio_dispatcher_thread
    - mxrio_handler
        - vfs_handler
            - vfs_handler_vn - this is the handler for mxrio_msg_t's to rootfs
    - vfs uses txn_handoff_open to forward open messages along to the right owner

   ERR_DISPATCHER_INDIRECT is a special error to say that the
      message was handed to another server who will do the replay
      themselves.
