build:
	# gcc -o run hello.c
	gcc -o out/vdev uinput-device.c -ludev