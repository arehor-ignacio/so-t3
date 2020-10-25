#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define DEVICE_NAME "nulo"

ssize_t null_read (struct file* file, char __user * user, size_t s, loff_t *off) {
	return 0;
}

ssize_t null_write (struct file* file, const char __user *user, size_t s, loff_t *off) {
	return s;
}

static struct cdev null_dev;
static struct class *null_class;
static struct file_operations null_fops = {
	.owner = THIS_MODULE,
	.read = null_read,
	.write = null_write,
};
static dev_t null_major;

static int __init hello_init(void) {
	printk(KERN_ALERT "Hola, Sistemas Operativos!\n");

	// Hacemos el init del char device
	cdev_init(&null_dev, &null_fops);

	// Obtenemos el major del módulo de manera dinámica
	if (alloc_chrdev_region(&null_major, 0, 1, DEVICE_NAME)) {
		printk(KERN_ALERT "Se rompió todo!");
	}

	// Asignamos major y minor a nuestro char device
	if (cdev_add(&null_dev, null_major, 1)) {
		printk(KERN_ALERT "Se rompió el cdev_add!");
	}

	// Solicitamos la creación de los nodos correspondientes al filesystem.
	null_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(null_class, NULL, null_major, NULL, DEVICE_NAME);

	return 0;
}

static void __exit hello_exit(void) {
	printk(KERN_ALERT "Descargando módulo...\n");

	// Destruímos el dispositivo
	device_destroy(null_class, null_major);
	class_destroy(null_class);

	// Desalojamos el major y el minor
	unregister_chrdev_region(null_major, 1);
	cdev_del(&null_dev);

	printk(KERN_ALERT "Adios, mundo cruel...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
