Magenta build instructions can be found in
[Getting Started](https://github.com/fuchsia-mirror/magenta/blob/master/docs/getting_started.md). The Fuchsia build system is documented
in [Build System](https://github.com/fuchsia-mirror/docs/blob/master/build_system.md).


# Adding Magenta Programs
Adding a new magenta user application to the build system is straightforward.
Create a new directory under a `uapp` directory, such as
`third_party/uapp` and add build rules and your source code. 
Here is a small test program named `hello.c`:

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
    printf("hello world!\n");

    char buf[16];
    int fd = open("/dev/zero", O_RDONLY);
    if(fd >= 0) {
        printf("got fd %d\n", fd);
        int x = read(fd, buf, sizeof buf);
        printf("read %d: ", x);
        for(int i = 0; i < x; i++) {
            printf("%02x ", (unsigned char)buf[i]);
        }
        printf("\n");
    }
    return 0;
}
```

This is the `rules.mk` file with build instructions:

```
LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
MODULE_TYPE := userapp
MODULE_SRCS += $(LOCAL_DIR)/hello.c
MODULE_NAME := hello
MODULE_LIBS := system/ulib/magenta system/ulib/mxio system/ulib/c
include make/module.mk
```

Once these files are in place, rebuild the system, and the
resulting program will be placed in the `/boot/bin` directory.
You can include extra data files by specifying manifest lines
in `rules.mk`:

```
USER_MANIFEST_LINES += src/hello.c=$(LOCAL_DIR)/hello.c
```

This will result in `hello.c` being installed as `/boot/src/hello.c`

