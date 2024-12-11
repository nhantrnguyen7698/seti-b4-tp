#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>  // Inclure le framework misc
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#define ADXL345_IOC_MAGIC 'a'
#define ADXL345_SET_AXIS _IOW(ADXL345_IOC_MAGIC, 1, int)

struct adxl345_sample {
    int16_t x;  // Valeur pour l'axe X
    int16_t y;  // Valeur pour l'axe Y
    int16_t z;  // Valeur pour l'axe Z
};

struct adxl345_device
{
    struct miscdevice miscdev;
    // FIFO pour stocker les échantillons
    DECLARE_KFIFO(samples_fifo, struct adxl345_sample, 64);
    wait_queue_head_t wait_queue;  // File d'attente pour les processus en sommeil
    int current_axis; // 0 = X, 1 = Y, 2 = Z
};

static int adxl345_count = 0;

static long adxl345_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct adxl345_device *dev = file->private_data;

    pr_info("ADXL345_IOCTL received cmd: 0x%x, arg: %lu\n", cmd, arg);

    switch (cmd) {
    case ADXL345_SET_AXIS:
        if (arg > 2) {
            pr_err("Invalid axis: %lu\n", arg);
            return -EINVAL;
        }
        dev->current_axis = arg;
        pr_info("ADXL345 axis set to %lu\n", arg);
        break;

    default:
        pr_err("Unknown command: 0x%x\n", cmd);
        return -ENOTTY; // Commande non supportée
    }

    return 0;
}

static ssize_t adxl345_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct adxl345_device *dev = container_of(file->private_data, struct adxl345_device, miscdev);
    struct i2c_client *client = to_i2c_client(dev->miscdev.parent);
    u8 reg_addr;
    u8 data[2];
    int ret;

    // Déterminer l'adresse du registre en fonction de l'axe
    switch (dev->current_axis) {
    case 0:
        reg_addr = 0x32; // DATAX0
        break;
    case 1:
        reg_addr = 0x34; // DATAY0
        break;
    case 2:
        reg_addr = 0x36; // DATAZ0
        break;
    default:
        return -EINVAL;
    }

    // Écrire l'adresse du registre à lire
    ret = i2c_master_send(client, &reg_addr, 1);
    if (ret != 1) {
        pr_err("Failed to send register address\n");
        return -EIO;
    }

    // Lire les données
    ret = i2c_master_recv(client, data, 2);
    if (ret != 2) {
        pr_err("Failed to receive data\n");
        return -EIO;
    }

    // Copier vers l'utilisateur
    if (count == 1) {
        if (copy_to_user(buf, &data[1], 1))
            return -EFAULT;
        return 1;
    } else {
        if (copy_to_user(buf, data, 2))
            return -EFAULT;
        return 2;
    }
}

static const struct file_operations adxl345_fops = {
    .owner = THIS_MODULE,
    // .open = adxl345_open,
    .read = adxl345_read,
    .unlocked_ioctl = adxl345_ioctl, // Déclarez la fonction ioctl
};

// static int adxl345_read_reg(struct i2c_client *client, u8 reg_addr, u8* value){
//     int ret;

//     // Écrire l'adresse du registre DEVID sur le bus I2C
//     ret = i2c_master_send(client, &reg_addr, 1);
//     if (ret < 0) {
//         pr_err("Failed to set register address to ADXL345: %d\n", ret);
//         return ret;
//     }

//     // Lire la valeur du registre DEVID
//     ret = i2c_master_recv(client, value, 1);
//     if (ret < 0) {
//         pr_err("Failed to read register from ADXL345: %d\n", ret);
//         return ret;
//     }
//     return 0;
// }

static int adxl345_read_fifo_sample(struct i2c_client *client, struct adxl345_sample *sample)
{
    u8 reg_addr = 0x32;  // Adresse du premier registre de données (DATAX0)
    u8 data[6];          // 6 octets pour X (2), Y (2) et Z (2)
    int ret;

    // Écrire l'adresse du registre DATAX0
    ret = i2c_master_send(client, &reg_addr, 1);
    if (ret != 1) {
        pr_err("Failed to set register address for FIFO read\n");
        return -EIO;
    }

    // Lire les 6 octets de la FIFO
    ret = i2c_master_recv(client, data, 6);
    if (ret != 6) {
        pr_err("Failed to read sample data from FIFO\n");
        return -EIO;
    }

    // Remplir la structure adxl345_sample avec les données reçues
    sample->x = (data[1] << 8) | data[0];  // DATAX1 (MSB) et DATAX0 (LSB)
    sample->y = (data[3] << 8) | data[2];  // DATAY1 (MSB) et DATAY0 (LSB)
    sample->z = (data[5] << 8) | data[4];  // DATAZ1 (MSB) et DATAZ0 (LSB)

    return 0;
}

// static irqreturn_t adxl345_irq_handler(int irq, void *dev_id)
// {
//     struct adxl345_device *dev = dev_id;
//     struct i2c_client *client = to_i2c_client(dev->miscdev.parent);
//     struct adxl345_sample sample;
//     u8 fifo_status;
//     int ret;

//     // Lire le registre FIFO_STATUS pour connaître le nombre d'échantillons
//     ret = adxl345_read_reg(client, 0x39, &fifo_status);
//     if (ret < 0) {
//         pr_err("Failed to read FIFO_STATUS: %d\n", ret);
//         return IRQ_NONE;
//     }

//     // Vider la FIFO matérielle dans la FIFO logicielle
//     while (fifo_status & 0x1F) { // FIFO Entries dans les 5 bits de poids faible
//         ret = adxl345_read_fifo_sample(client, &sample);
//         if (ret < 0) {
//             pr_err("Failed to read sample from FIFO\n");
//             break;
//         }

//         if (!kfifo_put(&dev->samples_fifo, sample))
//             pr_warn("Driver FIFO full, dropping sample\n");

//         fifo_status--;
//     }

//     // Réveiller les processus en attente
//     wake_up_interruptible(&dev->wait_queue);

//     return IRQ_HANDLED;
// }

irqreturn_t adxl345_int(int irq, void *dev_id) {
    struct adxl345_device *dev = (struct adxl345_device *)dev_id;
    struct i2c_client *client = to_i2c_client(dev->miscdev.parent);
    int ret, samples_count;
    struct adxl345_sample sample;

    // Lire le nombre d'échantillons disponibles dans FIFO_STATUS
    u8 fifo_status_reg = 0x39; // Adresse du registre FIFO_STATUS
    u8 fifo_status;
    ret = i2c_master_send(client, &fifo_status_reg, 1);
    if (ret != 1) {
        pr_err("Failed to send FIFO_STATUS register address\n");
        return IRQ_HANDLED;
    }
    ret = i2c_master_recv(client, &fifo_status, 1);
    if (ret != 1) {
        pr_err("Failed to read FIFO_STATUS\n");
        return IRQ_HANDLED;
    }
    samples_count = fifo_status & 0x3F; // Les 6 bits de poids faible

    // Lire tous les échantillons disponibles
    while (samples_count--) {
        ret = adxl345_read_fifo_sample(client, &sample);
        if (ret) {
            pr_err("Failed to read FIFO sample\n");
            break;
        }
        if (!kfifo_put(&dev->samples_fifo, sample)) {
            pr_err("Driver FIFO is full\n");
            break;
        }
    }

    // Réveiller les processus en attente
    wake_up(&dev->wait_queue);

    return IRQ_HANDLED;
}

static int adxl345_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct adxl345_device *dev;
    int ret;

    // Allouer la mémoire pour adxl345_device
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    // Associer la structure à i2c_client
    dev->miscdev.parent = &client->dev;
    i2c_set_clientdata(client, dev);

    // Initialiser la FIFO
    INIT_KFIFO(dev->samples_fifo);

    // Configurer la structure miscdevice
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev->miscdev.name = kasprintf(GFP_KERNEL, "adxl345-%d", adxl345_count++);
    if (!dev->miscdev.name) {
        kfree(dev);
        return -ENOMEM;
    }
    dev->miscdev.fops = &adxl345_fops;

    // Enregistrer le périphérique auprès du framework misc
    ret = misc_register(&dev->miscdev);
    if (ret) {
        pr_err("Failed to register misc device\n");
        kfree(dev->miscdev.name);
        kfree(dev);
        return ret;
    }

    // IITIALISATION DU CAPTEUR ADXL345

    // Configuration du registre BW_RATE (fréquence de sortie des données : 100 Hz)
    ret = i2c_smbus_write_byte_data(client, 0x2C, 0x0A); // 0x0A = 100 Hz
    if (ret) {
        pr_err("Failed to write BW_RATE register\n");
        goto err_misc_deregister;
    }

    // Activer l’interruption Watermark
    ret = i2c_smbus_write_byte_data(client, 0x2E, 0x04); // INT_ENABLE: activer uniquement l'interruption Watermark (bit [2])
    if (ret) {
        pr_err("Failed to write INT_ENABLE register\n");
        goto err_misc_deregister;
    }

    // Configuration du registre DATA_FORMAT (format de données par défaut)
    ret = i2c_smbus_write_byte_data(client, 0x31, 0x00); // 0x00 = format par défaut
    if (ret) {
        pr_err("Failed to write DATA_FORMAT register\n");
        goto err_misc_deregister;
    }

    // Configurer le registre FIFO_CTL en mode Stream et définir le Watermark
    ret = i2c_smbus_write_byte_data(client, 0x38, 0x1A); // FIFO_CTL: mode Stream (bits [6:5] = 10) et Watermark = 20
    if (ret) {
        pr_err("Failed to write FIFO_CTL register\n");
        goto err_misc_deregister;
    }

    // Configuration du registre POWER_CTL (mode mesure activé)
    ret = i2c_smbus_write_byte_data(client, 0x2D, 0x08); // 0x08 = mode mesure
    if (ret) {
        pr_err("Failed to write POWER_CTL register\n");
        goto err_misc_deregister;
    }

    // Initialiser la file d’attente pour la gestion des processus en sommeil
    init_waitqueue_head(&dev->wait_queue);

    // Enregistrer un gestionnaire d'interruption avec Threaded IRQ
    ret = devm_request_threaded_irq(&client->dev, client->irq, NULL,
                                    adxl345_int,
                                    IRQF_ONESHOT, "adxl345_irq", dev);
    if (ret) {
        pr_err("Failed to request IRQ: %d\n", ret);
        goto err_misc_deregister;
    }

    pr_info("ADXL345 initialized successfully: ");
    pr_info("ADXL345 misc device registered as %s\n", dev->miscdev.name);
    return 0;

err_misc_deregister:
    misc_deregister(&dev->miscdev);
    kfree(dev->miscdev.name);
    kfree(dev);
    return ret;
}

static int adxl345_remove(struct i2c_client *client)
{
    struct adxl345_device *dev;

    // Récupérer l'instance associée à i2c_client
    dev = i2c_get_clientdata(client);

    // Désactiver le capteur (mode veille)
    i2c_smbus_write_byte_data(client, 0x2D, 0x00); // 0x00 = mode veille

    // Désenregistrer le périphérique auprès du framework misc
    misc_deregister(&dev->miscdev);

    // Libérer les ressources
    kfree(dev->miscdev.name);
    kfree(dev);

    pr_info("ADXL345 misc device unregistered\n");
    return 0;
}

/* La liste suivante permet l'association entre un périphérique et son
   pilote dans le cas d'une initialisation statique sans utilisation de
   device tree.

   Chaque entrée contient une chaîne de caractère utilisée pour
   faire l'association et un entier qui peut être utilisé par le
   pilote pour effectuer des traitements différents en fonction
   du périphérique physique détecté (cas d'un pilote pouvant gérer
   différents modèles de périphérique).
*/
static struct i2c_device_id adxl345_idtable[] = {
    { "adxl345", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, adxl345_idtable);

#ifdef CONFIG_OF
/* Si le support des device trees est disponible, la liste suivante
   permet de faire l'association à l'aide du device tree.

   Chaque entrée contient une structure de type of_device_id. Le champ
   compatible est une chaîne qui est utilisée pour faire l'association
   avec les champs compatible dans le device tree. Le champ data est
   un pointeur void* qui peut être utilisé par le pilote pour
   effectuer des traitements différents en fonction du périphérique
   physique détecté.
*/
static const struct of_device_id adxl345_of_match[] = {
    { .compatible = "qemu,adxl345",
      .data = NULL },
    {}
};

MODULE_DEVICE_TABLE(of, adxl345_of_match);
#endif

static struct i2c_driver adxl345_driver = {
    .driver = {
        /* Le champ name doit correspondre au nom du module
           et ne doit pas contenir d'espace */
        .name   = "adxl345",
        .of_match_table = of_match_ptr(adxl345_of_match),
    },

    .id_table       = adxl345_idtable,
    .probe          = adxl345_probe,
    .remove         = adxl345_remove,
};

module_i2c_driver(adxl345_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adxl345 driver");
MODULE_AUTHOR("Trong Nhan NGUYEN");
