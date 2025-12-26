#include <linux/uinput.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <libudev.h>
#include <stdlib.h>

typedef struct
{
    char name[256];
    char event_path[256];
} device;

int count_devices(struct udev_list_entry *devices, struct udev *udev)
{
    struct udev_list_entry *entry;
    int entryCount = 0;
    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        if (dev)
        {
            entryCount++;
            udev_device_unref(dev);
        }
    }
    return entryCount;
}

device *get_devices(int *count)
{
    struct udev *udev = udev_new();
    if (!udev)
    {
        fprintf(stderr, "Failed to create udev context\n");
        *count = 0;
        return NULL;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_property(enumerate, "DEVNAME", "/dev/input/event*");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

    int entryCount = count_devices(devices, udev);

    device *udevices = malloc(entryCount * sizeof(device));
    if (!udevices)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        *count = 0;
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return NULL;
    }

    int i = 0;
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        if (dev)
        {
            struct udev_device *parent = udev_device_get_parent(dev);
            const char *name = parent ? udev_device_get_sysattr_value(parent, "name") : "(null)";
            strcpy(udevices[i].name, name ? name : "(null)");
            strcpy(udevices[i].event_path, udev_device_get_devnode(dev));
            i++;
            udev_device_unref(dev);
        }
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    *count = entryCount;
    return udevices;
}

int print_devices_by_reading_proc()
{
    FILE *fptr = fopen("/proc/bus/input/devices", "r");

    if (fptr == NULL)
    {
        return 1;
    }

    char ch;
    while ((ch = fgetc(fptr)) != EOF)
    {
        putchar(ch);
    }

    fclose(fptr);
    return 0;
}

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

int main(void)
{
    struct uinput_setup usetup;

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);  // Left stick X
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);  // Left stick Y
    ioctl(fd, UI_SET_ABSBIT, ABS_RX); // Right stick X
    ioctl(fd, UI_SET_ABSBIT, ABS_RY); // Right stick Y
    ioctl(fd, UI_SET_ABSBIT, ABS_Z);  // Left trigger
    ioctl(fd, UI_SET_ABSBIT, ABS_RZ); // Right trigger

    ioctl(fd, UI_SET_KEYBIT, BTN_TL);
    ioctl(fd, UI_SET_KEYBIT, BTN_TR);

    ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL);
    ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR);

    ioctl(fd, UI_SET_KEYBIT, BTN_A);
    ioctl(fd, UI_SET_KEYBIT, BTN_B);
    ioctl(fd, UI_SET_KEYBIT, BTN_X);
    ioctl(fd, UI_SET_KEYBIT, BTN_Y);

    ioctl(fd, UI_SET_KEYBIT, BTN_START);
    ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);

    struct uinput_abs_setup abs_setup;

    // Left stick X
    memset(&abs_setup, 0, sizeof(abs_setup));
    abs_setup.code = ABS_X;
    abs_setup.absinfo.minimum = -32767;
    abs_setup.absinfo.maximum = 32767;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    // Left stick Y
    abs_setup.code = ABS_Y;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    // Right stick X
    abs_setup.code = ABS_RX;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    // Right stick Y
    abs_setup.code = ABS_RY;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    // Left trigger (typically 0-255 for pressure)
    abs_setup.code = ABS_Z;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = 255;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    // Right trigger
    abs_setup.code = ABS_RZ;
    ioctl(fd, UI_ABS_SETUP, &abs_setup);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;

    usetup.id.vendor = 0x2dc8;
    usetup.id.product = 0x6001;
    // strcpy(usetup.name, "SN30/SF30 Pro gamepad");
    strcpy(usetup.name, "Virtual SN30/SF30 Pro gamepad");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    // printf("sleeping for 10 s\n");
    // sleep(10);

    emit(fd, EV_KEY, KEY_SPACE, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    emit(fd, EV_KEY, KEY_SPACE, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    printf("pressed and released space key\n");

    emit(fd, EV_KEY, BTN_A, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    usleep(2000);
    emit(fd, EV_KEY, BTN_A, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    printf("pressed and released A button\n");

    emit(fd, EV_KEY, BTN_B, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    emit(fd, EV_KEY, BTN_B, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    printf("pressed and released B button\n");

    // printf("sleeping for 10 s\n");
    // sleep(10);

    int device_count;
    device *devices = get_devices(&device_count);
    if (devices)
    {
        printf("found %d devices:\n", device_count);
        device d1 = devices[0];
        printf("%s;%s\n", d1.name, d1.event_path);

        // TODO

        free(devices);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
