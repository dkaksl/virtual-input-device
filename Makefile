build:
	mkdir -p out
	gcc -o out/vdev \
		src/uinput-device.c \
		src/udev.c \
		-ludev