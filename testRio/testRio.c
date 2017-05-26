/*
 * Example of how to perform remote IO calls to the filesystem.
 * The code intentionally does not abstract common patterns to be as
 * linear as possible.
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <magenta/syscalls.h>
#include <magenta/syscalls/object.h>

#include <mxio/remoteio.h>
#include "private-remoteio.h"  // copied from mxio library

/* steal a copy of mxio's root handle for ourselves */
mx_handle_t getRoot(void)
{
    mxrio_t *r = (mxrio_t*)mxio_root_handle;
    return r->h;
}

void
showHandle(mx_handle_t h)
{
    mx_info_handle_basic_t info;
    mx_status_t x;

    x = mx_object_get_info(h, MX_INFO_HANDLE_BASIC, &info, sizeof info, 0, 0);
    if(x != NO_ERROR) {
        printf("%x: bad handle\n", h);
    } else {
        printf("%x type %d rights %x koid %lx\n", h, info.type, info.rights, info.koid);
    }
}

void
dumpHex(const void *vp, size_t sz)
{
    const unsigned char *p = (const unsigned char *)vp;
    size_t i;

    while(sz > 0) {
        for(i = 0; i < 8 && i < sz; i++) {
            printf("%02x ", p[i]);
        }
        printf("\n");
        p += i;
        sz -= i;
    }
}

int main(int argc, char **argv)
{
    const char *fn = "/dev/zero";
    mx_status_t x;
    mx_handle_t root, h1, h2;

    if(argc > 1) fn = argv[1];

    /* 
     * Each process has a handle to the root filesystem.  Our libraries
     * know about it.  To make calls, we need to steal a copy.
     */
    root = getRoot();
    showHandle(root);

    /* to open a file we need a new channel pair */
    if(mx_channel_create(0, &h1, &h2) != NO_ERROR) {
        printf("cant get channel\n");
        return 1;
    }

    /* 
     * We build an OPEN call, passing in half of the channel pair.
     * We keep the other half and use it to talk to the server.
     * Note: the server on the far end of this channel may not necessarily
     * be the same as our root filesystem server.  The server can forward
     * the pair along to other servers as needed.
     */
    printf("path %s\n", fn);
    mxrio_msg_t msg;
    memset(&msg, 0, sizeof msg);
    msg.txid = 0x01010101;
    msg.op = MXRIO_OPEN;
    msg.datalen = strlen(fn);
    msg.arg = O_RDONLY;
    msg.arg2.mode = 0666;
    msg.hcount = 1;
    strcpy((char*)msg.data, fn);

    mx_handle_t handtab[4];
    handtab[0] = h2;

    /* send the message */
    x = mx_channel_write(root, 0, &msg, MXRIO_HDR_SZ + msg.datalen, handtab, msg.hcount);
    if(x != NO_ERROR) {
        printf("write failed %d\n", x);
        return 1;
    }
    printf("wrote\n");

    /* wait for a response from h1 (not root!) */
    x = mx_object_wait_one(h1, MX_CHANNEL_READABLE, MX_TIME_INFINITE, 0);
    if(x != NO_ERROR) {
        printf("wait failed %d\n", x);
        return 1;
    }

    /* 
     * Read the response from h1.  It will have a status code
     * and info about the type of connection.  We expect status == NO_ERROR
     * and type == MXIO_PROTOCOL_REMOTE.
     */
    uint32_t nb, nh;
    mxrio_object_t obj;
    x = mx_channel_read(h1, 0, &obj, handtab, sizeof obj,
                        sizeof handtab / sizeof handtab[0], &nb, &nh);
    if(x != NO_ERROR) {
        printf("read failed %d\n", x);
        return 1;
    }
    if(nb < 8) {
        printf("short read %d\n", nb);
        return 1;
    }
    printf("status %d type %d\n", obj.status, obj.type);
    if(obj.status != 0 || obj.type != 2) { // remote object
        printf("not what I expected!\n");
        return 1;
    }

    /* we have our open handle now, so lets do a read operation */
    memset(&msg, 0, sizeof msg);
    msg.txid = 0x02020202;
    msg.op = MXRIO_READ_AT;
    msg.arg = 8;
    msg.arg2.off = 0;
    x = mx_channel_write(h1, 0, &msg, MXRIO_HDR_SZ + msg.datalen, 0, 0);
    if(x != NO_ERROR) {
        printf("write2 failed %d\n", x);
        return 1;
    }

    /* 
     * We have to wait for a response.  We could have used
     * mx_channel_call here to writ, wait and read all at once.
     */
    x = mx_object_wait_one(h1, MX_CHANNEL_READABLE, MX_TIME_INFINITE, 0);
    if(x != NO_ERROR) {
        printf("wait2 failed %d\n", x);
        return 1;
    }

    /* read the result (also a msg) */
    x = mx_channel_read(h1, 0, &msg, handtab, sizeof msg,
                        sizeof handtab / sizeof handtab[0], &nb, &nh);
    if(x != NO_ERROR) {
        printf("read2 failed %d\n", x);
        return 1;
    }
    if(nb < MXRIO_HDR_SZ) {
        printf("short read2 %d\n", nb);
        return 1;
    }

    /* We have our read response. Print out the contents. */
    printf("data len %d\n", msg.datalen);
    printf("data: ");
    dumpHex(msg.data, msg.datalen);

    return 0;
}
