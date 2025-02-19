#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

void read_sensor(const char *device) {
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }

    char buffer[16];
    int ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0) {
        perror("Read failed");
    } else {
        printf("Data from %s: ", device);
        for (int i = 0; i < ret; i++) {
            printf("%02x ", buffer[i]);
        }
        printf("\n");
    }

    close(fd);
}

int main() {
    printf("Reading from ADXL345 sensors...\n");
    read_sensor("/dev/adxl345-0");
    read_sensor("/dev/adxl345-1");
    return 0;
}
