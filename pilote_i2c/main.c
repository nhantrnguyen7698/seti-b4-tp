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
    printf("HELLO WORLD\n");
    int fd = open("/dev/adxl345-0", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    printf("HELLO WORLD-222222222222\n");
    struct adxl345_sample sample;
    while (1) {
        printf("HELLO WORLD-33333333333333\n");
        int ret = read(fd, &sample, sizeof(sample));
        printf("HELLO WORLD-44444444444444\n");
        if (ret < 0) {
            perror("Failed to read data");
            close(fd);
            return -1;
        }

        printf("X: %d, Y: %d, Z: %d\n", sample.x, sample.y, sample.z);
        usleep(50000); // Attendre 500 ms pour lire les données suivantes
    }

    close(fd);
    return 0;
}
