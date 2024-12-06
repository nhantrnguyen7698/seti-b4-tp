#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>  // Inclure le framework misc
#include <linux/fs.h>

struct adxl345_device
{
    struct miscdevice miscdev;
};

static int adxl345_count = 0;

// static int adxl345_open(struct inode *inode, struct file *file)
// {
//     struct adxl345_device *dev = container_of(inode->i_cdev, struct adxl345_device, miscdev.this_device->devt);
//     if (!dev)
//         return -ENODEV;

//     file->private_data = dev;  // Initialiser private_data
//     return 0;
// }

static ssize_t adxl345_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct adxl345_device *dev = container_of(file->private_data, struct adxl345_device, miscdev);
    struct i2c_client *client = to_i2c_client(dev->miscdev.parent);
    u8 reg_addr = 0x32;  // Adresse DATAX0
    u8 data[2];
    int ret;

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
};

static int adxl345_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct adxl345_device *dev;
    int ret;

    // // INITIALIZATION
    // pr_info("ADXL345 detected at address 0x%x\n", client->addr);

    // // Configurer les registres de l'accéléromètre
    // ret = adxl345_write_reg(client, 0x2C, 0x0A);  // BW_RATE: 100 Hz
    // if (ret < 0) return ret;

    // ret = adxl345_write_reg(client, 0x2E, 0x00);  // INT_ENABLE: Désactive les interruptions
    // if (ret < 0) return ret;

    // ret = adxl345_write_reg(client, 0x31, 0x00);  // DATA_FORMAT: Format par défaut
    // if (ret < 0) return ret;

    // ret = adxl345_write_reg(client, 0x38, 0x00);  // FIFO_CTL: Mode bypass
    // if (ret < 0) return ret;

    // ret = adxl345_write_reg(client, 0x2D, 0x08);  // POWER_CTL: Mode mesure
    // if (ret < 0) return ret;
    // // END OF INITIALIZATION

    // Allouer la mémoire pour adxl345_device
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    // Associer la structure à i2c_client
    dev->miscdev.parent = &client->dev;
    i2c_set_clientdata(client, dev);
    // dev = container_of(inode->i_cdev, struct adxl345_device, miscdev.this_device->devt)

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

    pr_info("ADXL345 misc device registered as %s\n", dev->miscdev.name);
    return 0;
}

static int adxl345_remove(struct i2c_client *client)
{
    struct adxl345_device *dev;

    // Récupérer l'instance associée à i2c_client
    dev = i2c_get_clientdata(client);

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
