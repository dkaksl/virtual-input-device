#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udev.h"

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