
typedef struct
{
    char name[256];
    char event_path[256];
} device;

int count_devices(struct udev_list_entry *devices, struct udev *udev);

device *get_devices(int *count);