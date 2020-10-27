#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/random.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define DEVICE_NAME "letras123"
#define CANT_ESPACIOS 3
#define SUCCESS 0

struct semaphore mutex;

char letras[CANT_ESPACIOS] = {0};
int ocupados[CANT_ESPACIOS] = {0};
int escritos[CANT_ESPACIOS] = {0};

ssize_t read (struct file* file, char __user * data, size_t size, loff_t *off) {
	if (file->private_data == NULL || escritos[*((int*)file->private_data)] != 1) {
		return -EPERM;
	}

	int i = 0;
	while (i < size && copy_to_user(data + i, letras + *((int*)file->private_data), 1) == 0) {
		i++;
	}

	if (i < size) {
		printk(KERN_ALERT "No tienes permisos loko!");
		return 0;
	}

	return size;
}

ssize_t write (struct file* file, const char __user * data, size_t size, loff_t *off) {
	if (file->private_data == NULL) {
		return -EPERM;
	}

	if (copy_from_user(letras + *((int*)file->private_data), data, 1) != 0) {
		printk(KERN_ALERT "No tienes permisos loko!");
	}

	escritos[*((int*)file->private_data)] = 1;
	return size;
}

ssize_t open (struct inode* inode, struct file* file) {
	down(&mutex);

	int i = 0;
	while (i < CANT_ESPACIOS && ocupados[i] != NULL) {
		i++;
	}

	if (i == CANT_ESPACIOS) {
		printk(KERN_ALERT "Todos los lugares llenos.");
	} else {
		ocupados[i] = 1;
		file->private_data = kmalloc(sizeof(int), GFP_KERNEL);
		*((int*)file->private_data) = i;
	}

	up(&mutex);

	return i < CANT_ESPACIOS ? SUCCESS : -EPERM;
}

ssize_t release (struct inode* inode, struct file* file) {
	if (file->private_data == NULL) {
		return -EPERM;
	}

	ocupados[*((int*)file->private_data)] = NULL;
	escritos[*((int*)file->private_data)] = 0;
	kfree(file->private_data);

	return SUCCESS;
}

static struct cdev _dev;
static struct class *_class;
static struct file_operations _fops = {
	.owner = THIS_MODULE,
	.read = read,
	.write = write,
	.open = open,
	.release = release,
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

	sema_init(&mutex, 1);

	return 0;
}

static void __exit hello_exit(void) {
	printk(KERN_ALERT "Descargando mod...\n");

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
