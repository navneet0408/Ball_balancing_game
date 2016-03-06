#include <linux/module.h>  // Module Defines and Macros (THIS_MODULE)
#include <linux/kernel.h>  // 
#include <linux/slab.h>	   // Kmalloc/Kfree
#include <linux/i2c.h>     // i2c Kernel Interfaces
#include <linux/i2c-dev.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/time.h>

// #defines...
#define DEVICE_NAME					"MPU"
#define SENSOR_SLAVE_ADDRESS  		0X68
#define ACC_REGISTER				0x3B
#define GYRO_REGISTER				0x43

#define I2C_PORT					29


//Global structs and variables
static struct input_dev *joystick_dev;
struct MPU_dev
{
	long xAngle, yAngle, zAngle;
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	long elapsedTimeT1;
	long elapsedTimeT2;
	long elapsedTimeT3;
	long elapsedTimeT4;
	long iterations;
} *MPU_devp;
static struct task_struct *readThread = NULL;

static long Microsecs()
{
   struct timespec t;  
   getnstimeofday(&t);  
   return (long)t.tv_sec*1000*1000 + (long)t.tv_nsec/1000;
}

//Kernel Thread to read data from MPU
static int readMPU(void* data)
{
	struct MPU_dev *mpu = (struct MPU_dev*) data;
	int ret=0;
	char ACC_readings[6]={0,0,0,0,0,0};
	char GYRO_readings[6] = {0,0,0,0,0,0};
	long xGyroReading, yGyroReading, zGyroReading;
	long xAccReading, yAccReading, zAccReading;
	long curTime;
	
	while(!kthread_should_stop())
	{
		//Read Accelerometer
		curTime=Microsecs();
		ACC_readings[0]=ACC_REGISTER;
		ret = i2c_master_send(mpu->client, ACC_readings,1);
		if(ret < 0)
		{
			printk("Error couldnt set addr.\n");
			return -1;
		}
		mpu->elapsedTimeT1+=(Microsecs()-curTime);
		
		msleep(1);
		
		curTime=Microsecs();
		ret = i2c_master_recv(mpu->client, ACC_readings,6);
		if(ret < 0)
		{
			printk("Error: could not read register.\n");
			return -1;
		}
		mpu->elapsedTimeT2+=(Microsecs()-curTime);

		msleep(1);

		//Read Gyrometer
		
		curTime=Microsecs();
		GYRO_readings[0]=GYRO_REGISTER;
		ret = i2c_master_send(mpu->client, GYRO_readings,1);
		if(ret < 0)
		{
			printk("Error couldnt set addr.\n");
			return -1;
		}
		mpu->elapsedTimeT3+=(Microsecs()-curTime);

		msleep(1);
		
		curTime=Microsecs();
		ret = i2c_master_recv(mpu->client, GYRO_readings,6);
		if(ret < 0)
		{
			printk("Error: could not read register.\n");
			return -1;
		}

		xAccReading = (long)(ACC_readings[0]*256+ACC_readings[1]);
		yAccReading = (long)(ACC_readings[2]*256+ACC_readings[3]);
		zAccReading = (long)(ACC_readings[4]*256+ACC_readings[5]);
		xGyroReading = (long)(GYRO_readings[0]*256+GYRO_readings[1]);
		yGyroReading = (long)(GYRO_readings[2]*256+GYRO_readings[3]);
		zGyroReading = (long)(GYRO_readings[4]*256+GYRO_readings[5]);
		
		input_report_abs(joystick_dev, ABS_X, xAccReading);
		input_report_abs(joystick_dev, ABS_Y, yAccReading);
		input_report_abs(joystick_dev, ABS_Z, zAccReading);
		input_report_abs(joystick_dev, ABS_RX, xGyroReading);
		input_report_abs(joystick_dev, ABS_RY, yGyroReading);
		input_report_abs(joystick_dev, ABS_RZ, zGyroReading);
		input_sync(joystick_dev);

		mpu->elapsedTimeT4+=(Microsecs()-curTime);
		msleep(7);
		//printk(" %d %d %d \n",i2c_gyro->deg_x*10/328, i2c_gyro->deg_y*10/328, i2c_gyro->deg_z*10/328);
		mpu->iterations++;
		if(mpu->iterations==100000000)
		{
			printk("Task 1: %ld", mpu->elapsedTimeT1);
			printk("Task 2: %ld", mpu->elapsedTimeT2);
			printk("Task 3: %ld", mpu->elapsedTimeT3);
			printk("Task 4: %ld", mpu->elapsedTimeT4);
			printk("ITERATINS: %ld", mpu->iterations);
		}
	}

	return 0;
}

// INIT FUNCTION
int __init MPU_device_init(void)
{
	int ret;
	char buf[2];
	//MEMORY ALLOCATION 
	MPU_devp = kmalloc(sizeof(struct MPU_dev),GFP_KERNEL);
	MPU_devp->elapsedTimeT1=0.0;
	MPU_devp->elapsedTimeT2=0.0;
	MPU_devp->elapsedTimeT3=0.0;
	MPU_devp->elapsedTimeT4=0.0;
	MPU_devp->iterations=0;


	ret = gpio_request(I2C_PORT,"I2C_PORT");
	if(ret)
	{
		printk("I2C Port Request failed\n");
		return -1;
	}	

	//SET DIRECTION ON PORT TO OUTPUT
	ret = gpio_direction_output(I2C_PORT,0);
	if(ret)
	{
		printk("Port couldnot be set to output\n");
		return -1;
	}

	//SET VALUE FOR OUTPUT PORT
	gpio_set_value_cansleep(I2C_PORT,0);

	MPU_devp->adapter = i2c_get_adapter(0);
	MPU_devp->client = kmalloc(sizeof(struct i2c_client),GFP_KERNEL);
	
	MPU_devp->client->addr = SENSOR_SLAVE_ADDRESS;
	MPU_devp->client->adapter = MPU_devp->adapter;
	
	snprintf(MPU_devp->client->name, 20, "MPU60X0");

	buf[0] =0x6B; buf[1] = 0x05;
	ret = i2c_master_send(MPU_devp->client, buf,2);
	if(ret < 0)
	{
		printk("Error: could not set sleep and power.\n");
		return -1;
	}
	
	buf[0]=0x1B;buf[1]=0x00;
	ret = i2c_master_send(MPU_devp->client, buf,2);
	if(ret < 0)
	{
		printk("Error: could not gyroscope config.\n");
		return -1;
	}

	buf[0]=0x1C;buf[1]=0x00;
	ret = i2c_master_send(MPU_devp->client, buf,2);
	if(ret < 0)
	{
		printk("Error: could not accelerometer config.\n");
		return -1;
	}

	printk("Sensor setup SUCCESSFUL\n");

	joystick_dev = input_allocate_device();
	joystick_dev->name = "Joystick";
    joystick_dev->phys = "A/Fake/Path";
    joystick_dev->id.bustype = BUS_HOST;
    joystick_dev->id.vendor = 0x0001;
    joystick_dev->id.product = 0x0001;
    joystick_dev->id.version = 0x0100;

    set_bit(EV_ABS,joystick_dev->evbit);
    set_bit(ABS_X, joystick_dev->keybit);
    set_bit(ABS_Y, joystick_dev->keybit);
    set_bit(ABS_Z, joystick_dev->keybit);
    set_bit(ABS_RX, joystick_dev->keybit);
    set_bit(ABS_RY, joystick_dev->keybit);
    set_bit(ABS_RZ, joystick_dev->keybit);


    input_set_abs_params(joystick_dev, ABS_X,-16384,16384,0,0);
    input_set_abs_params(joystick_dev, ABS_Y,-16384,16384,0,0);
    input_set_abs_params(joystick_dev, ABS_Z,-16384,16384,0,0);
    input_set_abs_params(joystick_dev, ABS_RX,-16384,16384,0,0);
    input_set_abs_params(joystick_dev, ABS_RY,-16384,16384,0,0);
    input_set_abs_params(joystick_dev, ABS_RZ,-16384,16384,0,0);

    ret = input_register_device(joystick_dev);
   	if(ret < 0)
	{
		printk("Error: could not register input.\n");
		return -1;
	}
  
	readThread = kthread_run(readMPU ,MPU_devp, "READ_MPU");
	return 0;
}

// EXIT FUNCTION 	
void __exit MPU_dev_exit(void)
{
	if(readThread)
	{
		printk("stop MyThread\n");
		kthread_stop(readThread);
	}
	msleep(1000);
	input_unregister_device(joystick_dev);
	i2c_put_adapter(MPU_devp->adapter);
	kfree(MPU_devp->client);
	kfree(MPU_devp);
		
	gpio_free(I2C_PORT);
	printk("Task 1: %ld", MPU_devp->elapsedTimeT1);
	printk("Task 2: %ld", MPU_devp->elapsedTimeT2);
	printk("Task 3: %ld", MPU_devp->elapsedTimeT3);
	printk("Task 4: %ld", MPU_devp->elapsedTimeT4);
	printk("ITERATINS: %ld", MPU_devp->iterations);

	printk("EXIT SUCCESSFUL \n");

}


module_init(MPU_device_init);
module_exit(MPU_dev_exit);
MODULE_LICENSE("GPL v2");
