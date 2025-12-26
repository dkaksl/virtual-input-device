#include <linux/uinput.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <libudev.h>
#include <stdlib.h>
#include "udev.h"

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
