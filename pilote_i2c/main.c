#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

int main() {
    int fd;
    uint8_t buffer[2];
    ssize_t ret;

    // Ouvrir le fichier spécial dans /dev/
    fd = open("/dev/adxl345-0", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Lire 2 octets (un échantillon complet de l'axe X)
    ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }

    // Afficher la valeur lue
    int16_t x_value = (buffer[1] << 8) | buffer[0];  // Convertir en entier signé 16 bits
    printf("X-axis value: %d\n", x_value);

    // Fermer le fichier
    close(fd);

    return EXIT_SUCCESS;
}