
#include <stdio.h>
#include <magenta/syscalls.h>
#include <magenta/syscalls/object.h>

const char *typeName(uint32_t type) 
{
    switch(type) {
    case MX_OBJ_TYPE_NONE: return "NONE";
    case MX_OBJ_TYPE_PROCESS: return "PROCESS";
    case MX_OBJ_TYPE_THREAD: return "THREAD";
    case MX_OBJ_TYPE_VMEM: return "VMEM";
    case MX_OBJ_TYPE_CHANNEL: return "CHANNEL";
    case MX_OBJ_TYPE_EVENT: return "EVENT";
    case MX_OBJ_TYPE_IOPORT: return "IOPORT";
    case MX_OBJ_TYPE_INTERRUPT: return "INTERRUPT";
    case MX_OBJ_TYPE_IOMAP: return "IOMAP";
    case MX_OBJ_TYPE_PCI_DEVICE: return "PCI_DEVICE";
    case MX_OBJ_TYPE_LOG: return "LOG";
    case MX_OBJ_TYPE_WAIT_SET: return "WAIT_SET";
    case MX_OBJ_TYPE_SOCKET: return "SOCKET";
    case MX_OBJ_TYPE_RESOURCE: return "RESOURCE";
    case MX_OBJ_TYPE_EVENT_PAIR: return "EVENT_PAIR";
    case MX_OBJ_TYPE_JOB: return "JOB";
    case MX_OBJ_TYPE_VMAR: return "VMAR";
    case MX_OBJ_TYPE_FIFO: return "FIFO";
    case MX_OBJ_TYPE_IOPORT2: return "IOPORT2";
    case MX_OBJ_TYPE_HYPERVISOR: return "HYPERVISOR";
    case MX_OBJ_TYPE_GUEST: return "GUEST";
    default: return "???";
    }
}

void getHandle(mx_handle_t h)
{
    mx_info_handle_basic_t info;
    mx_status_t x;

    x = mx_object_get_info(h, MX_INFO_HANDLE_BASIC, &info, sizeof info, 0, 0);
    if(x == NO_ERROR) {
        printf("%x type %s/%d rights %x koid %lx\n", h, typeName(info.type), info.type, info.rights, info.koid);
    }
}

int main(int argc, char **argv)
{
    mx_handle_t h1, h2, min, max, i;

    // handles are xor'd against a hidden value.  get one handle
    // to get an approximate range
    h1 = 0;
    if(mx_channel_create(0, &h1, &h2) == NO_ERROR) {
        printf("h1 is %x and h2 is %x\n", h1, h2);
    }

    min = h1 & ~0xffffff;
    max = min + 0xffffff;
    printf("search %x - %x\n", min, max);
    for(i = min; i < max; i+=2) {
        getHandle(i);
    }
    return 0;
}
