#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/random.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define DEVICE_NAME "azar"

int numero = -1;

ssize_t read (struct file* file, char __user * data, size_t size, loff_t *off) {
	if (numero < 0) {
		printk(KERN_ALERT "No llenaste el buffers loko!\n");
		return -EPERM;
	}

	// Generamos el número aleatorio
	uint r_int;
	get_random_bytes(&r_int, sizeof(uint));

	snprintf(data, sizeof(uint) + 2, "%d\n", r_int % numero);
	return sizeof(uint);
}

ssize_t write (struct file* file, const char __user * data, size_t size, loff_t *off) {
	char* str = kmalloc(size + 1, GFP_KERNEL);

	// Leemos del buffer y copiamos a str.
	if (copy_from_user(str, data, size) != 0) {
		printk(KERN_ALERT "No tenés permisos papá!");
	}
	str[size] = '\0';

	// Lo convertimos a int y lo almacenamos
	if (kstrtoint(str, 0, &numero) != 0) {
		return -EPERM;
	}

	kfree(str);
	return size;
}

static struct cdev _dev;
static struct class *_class;
static struct file_operations _fops = {
	.owner = THIS_MODULE,
	.read = read,
	.write = write,
};
static dev_t _major;

static int __init hello_init(void) {
	printk(KERN_ALERT "Hola, Sistemas Operativos!\n");

	// Hacemos el init del char device
	cdev_init(&_dev, &_fops);

	// Obtenemos el major del módulo de manera dinámica
	if (alloc_chrdev_region(&_major, 0, 1, DEVICE_NAME)) {
		printk(KERN_ALERT "Se rompió todo!");
	}

	// Asignamos major y minor a nuestro char device
	if (cdev_add(&_dev, _major, 1)) {
		printk(KERN_ALERT "Se rompió el cdev_add!");
	}

	// Solicitamos la creación de los nodos correspondientes al filesystem.
	_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(_class, NULL, _major, NULL, DEVICE_NAME);

	return 0;
}

static void __exit hello_exit(void) {
	printk(KERN_ALERT "Descargando módulo...\n");

	// Destruímos el dispositivo
	device_destroy(_class, _major);
	class_destroy(_class);

	// Desalojamos el major y el minor
	unregister_chrdev_region(_major, 1);
	cdev_del(&_dev);

	printk(KERN_ALERT "Adios, mundo cruel...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de 'Hola, mundo'");
