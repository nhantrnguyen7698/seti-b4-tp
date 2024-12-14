#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct adxl345_sample {
    short x;
    short y;
    short z;
};

int main() {
    int fd = open("/dev/adxl345-0", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    struct adxl345_sample sample;
    while (1) {
        int ret = read(fd, &sample, sizeof(sample));
        if (ret < 0) {
            perror("Failed to read data");
            close(fd);
            return -1;
        }
        printf("X: %d, Y: %d, Z: %d\n", sample.x, sample.y, sample.z);
        usleep(500000); // Attendre 500 ms pour lire les données suivantes
    }

    close(fd);
    return 0;
}
