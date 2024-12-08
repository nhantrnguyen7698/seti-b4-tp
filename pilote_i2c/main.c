#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define ADXL345_IOC_MAGIC 'a'
#define ADXL345_SET_AXIS _IOW(ADXL345_IOC_MAGIC, 1, int)

int main() {
    int fd = open("/dev/adxl345-0", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    int axis = 1; // Par exemple, sélectionner l'axe Y
    printf("Sending ioctl cmd: 0x%x, arg: %d\n", ADXL345_SET_AXIS, axis);
    if (ioctl(fd, ADXL345_SET_AXIS, axis) < 0) {
        perror("ioctl failed");
        close(fd);
        return -1;
    }

    printf("Axis set to %d\n", axis);
    close(fd);
    return 0;
}
