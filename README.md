# Guide d'installation et de démonstration

## Étape 1 : Télécharger les fichiers nécessaires

- **Télécharger et décompresser le package de compilation du noyau Linux** :  
  [linux_build.tar.xz](https://perso.telecom-paristech.fr/duc/cours/linux/data/linux_build.tar.xz)

- **Télécharger l'image d'un système de fichiers mémoire construit avec Buildroot, basé sur BusyBox** :  
  [rootfs.cpio.gz](https://perso.telecom-paristech.fr/duc/cours/linux/data/rootfs.cpio.gz)

- **Télécharger QEMU pour ARM** :  
  [qemu-system-arm](https://www.qemu.org/)


## Étape 2 : Lancer la démonstration

Pour démarrer la distribution Linux sur le système, exécutez la commande suivante :

```bash
./qemu-system-arm -nographic -machine vexpress-a9 \
-kernel linux-5.10.19/build/arch/arm/boot/zImage \
-dtb linux-5.10.19/build/arch/arm/boot/dts/vexpress-v2p-ca9.dtb \
-initrd rootfs.cpio.gz
```

## Étape 3 : Quitter QEMU

Pour quitter QEMU, utilisez la combinaison de touches suivante :

```bash
Ctrl-a x
```