#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/input.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <time.h>


#define KEYFILE "/dev/input/event2"

int fd2,gfd1,gfd2,gfd3;
double currentAngleX=0.0, currentAngleY=0.0;
long elapsedTimeT1 = 0;
long elapsedTimeT2 = 0;
long iterationsT1=0;
long iterationsT2=0;


long Microsecs()
{
   struct timeval t;
   struct timezone tz; 
   gettimeofday(&t, &tz);  
   return (long)t.tv_sec*1000*1000 + (long)t.tv_usec;
}

void gpio_initialize()
{
	gfd1 = open("/sys/class/gpio/export", O_WRONLY);
	write(gfd1, "42",2);
	close(gfd1); 
	// Set GPIO Direction
	gfd1 = open("/sys/class/gpio/gpio42/direction", O_WRONLY);
	write(gfd1, "out", 3);
	close(gfd1);
	
	gfd1 = open("/sys/class/gpio/gpio42/value", O_WRONLY);
	write(gfd1, "0", 1);


	gfd2 = open("/sys/class/gpio/export", O_WRONLY);
	write(gfd2, "43", 2);
	close(gfd2); 
	// Set GPIO Direction
	gfd2 = open("/sys/class/gpio/gpio43/direction", O_WRONLY);
	write(gfd2, "out", 3);
	close(gfd2);
	
	gfd2 = open("/sys/class/gpio/gpio43/value", O_WRONLY);
	write(gfd2, "0", 1);
	

	gfd3 = open("/sys/class/gpio/export", O_WRONLY);
	write(gfd3, "55", 2);
	close(gfd3); 
	// Set GPIO Direction
	gfd3 = open("/sys/class/gpio/gpio55/direction", O_WRONLY);
	write(gfd3, "out", 3);
	close(gfd3);

	gfd3 = open("/sys/class/gpio/gpio55/value", O_WRONLY);
	write(gfd3, "0", 1);
}


void led_grid_initialize()
{
	struct spi_ioc_transfer xfer;
	char writeBuf[2];
	xfer.rx_buf = (unsigned int)NULL;
	xfer.len = 2; // Bytes to send
	xfer.cs_change = 0;
	xfer.delay_usecs = 0;
	xfer.speed_hz = 10000000; //Setting Frequency as 10Mhz
	xfer.bits_per_word = 8;
	
	// Decoding BCD
	writeBuf[0] = 0x09; 
	writeBuf[1] = 0x00;
	xfer.tx_buf = (unsigned int)writeBuf;
	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");	 
	//Brightness 
	writeBuf[0] = 0x0a;  
	writeBuf[1] = 0x03; 
	xfer.tx_buf = (unsigned int)writeBuf;
	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");	
	//Scan Limit
	writeBuf[0]=0x0b; 
	writeBuf[1]=0x07;
	xfer.tx_buf = (unsigned int)writeBuf;
	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");	
	// Power Down mode
	writeBuf[0] = 0x0c;
	writeBuf[1]= 0x01;
	xfer.tx_buf = (unsigned int)writeBuf;
	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");	
	writeBuf[0] = 0x0f;
	writeBuf[1]= 0x00;
	xfer.tx_buf = (unsigned int)writeBuf;
	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");		
}

void signal_handler(int sig)
{
	if (sig == SIGINT)
    printf("received SIGINT\n");
	close(gfd1);
	close(gfd2);
	close(gfd3);
	close(fd2);
	printf ("Elapsed Time T1: %ld\n", elapsedTimeT1);
	printf ("Elapsed Time T2: %ld\n", elapsedTimeT2);
	printf ("Iterations T1: %ld\n", iterationsT1);
	printf ("Iterations T2: %ld\n", iterationsT2);
	printf("Exiting :)\n");
	exit(0);
}


void write_LED(char *data)
{
	int i;
	struct spi_ioc_transfer xfer;
	char writeBuf[2];
	xfer.rx_buf = (unsigned int)NULL;
	xfer.len = 2; // Bytes to send
	xfer.cs_change = 0;
	xfer.delay_usecs = 0;
	xfer.speed_hz = 10000000; //Setting Frequency as 10Mhz
	xfer.bits_per_word = 8;

	for (i=0; i<8; ++i)
	{
		
    	writeBuf[0]=i+1;
    	writeBuf[1]=data[i];
    	xfer.tx_buf = (unsigned int)writeBuf;
    	if(ioctl(fd2, SPI_IOC_MESSAGE(1), &xfer) < 0)
			perror("SPI Message Send Failed");


	}
}


void display_dot(char x,char y)
{
	int i;
	char data[8];

	for (i=0; i<8; ++i)
	{
		if(i != (x-1))
		{
			data[i] = 0x00;
		}
		else
		{
			data[i] = (0x01<<(y-1));
		}
	}

	write_LED(data);
}

void display_dest()
{
	char data[] ={0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
	char zero[] ={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;
	for (i=0; i<5;++i)
	{
		write_LED(data);
		usleep(200000);
		write_LED(zero);
		usleep(200000);
	}
}
void display_x()
{
	char data[] ={0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
	char zero[] ={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;
	for (i=0; i<5;++i)
	{
		write_LED(data);
		usleep(200000);
		write_LED(zero);
		usleep(200000);
	}
}

void display_fire()
{
	int i;
	char data[4][8] = {
						{0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
						{0x00, 0x00, 0x3c, 0x24, 0x24, 0x3c, 0x00, 0x00},
						{0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00},
						{0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff}
					};
	
	for (i=0; i<4;++i)
	{
		write_LED(data[i]);
		usleep(100000);
	}
}

void complementaryFilter(long xGyroReading, long yGyroReading, long zGyroReading,
	 						long xAccReading, long yAccReading, long zAccReading,
	 							double *pitch, double *roll)
{
    float pitchAcc, rollAcc;               
    
    *pitch += ((float)xGyroReading / 13100.0); // Angle around the X-axis
    *roll -= ((float)yGyroReading/ 13100.0);    // Angle around the Y-axis
 
    int forceMagnitudeApprox = abs(xAccReading) + abs(yAccReading) + abs(zAccReading);
    if (forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
    {
        pitchAcc = atan2f((float)yAccReading, (float)zAccReading) * 180 / M_PI;
        *pitch = *pitch * 0.98 + pitchAcc * 0.02;
        rollAcc = atan2f((float)xAccReading, (float)zAccReading) * 180 / M_PI;
        *roll = *roll * 0.98 + rollAcc * 0.02;
    }
} 

void *moveLed (void *data)
{
	double x= (double) (rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2;
	double y= (double) (rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2;
	char current_x=(char) x + 1;
	char current_y=(char) y + 1;
	int level=1, i;
	int timer_on=0;
	int threshold = 1;
	time_t centered_time;
	long curTime;

	currentAngleX = 0;
	currentAngleY = 0;

	while (1)
	{
		curTime = Microsecs();
		if(current_x<1 || current_y<1 || current_x>8 || current_y>8)
		{
			display_x();
			sleep(1);
			display_fire();
			x=(rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2; 
			y=(rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2;
			current_x=(char) x + 1;
			current_y=(char) y + 1;
			display_dot(current_x,current_y);
			currentAngleX = currentAngleY = 0;
			threshold=1;
			level=1;
			sleep(1);
			elapsedTimeT2-=4400000;
		}
		
		if((current_y==4 || current_y==5) && (current_x==4 || current_x==5))
		{
			if(timer_on==1)
			{
				if((time(NULL) - centered_time) > 3)
				{
					timer_on=0;
					level++;
					for (i=0; i<level; ++i)
					{
						display_fire();
					}
					sleep(1);
					x=(rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2; 
					y=(rand() %2 == 0) ? 1+ rand()%2 : 5+rand()%2;
					current_x=(char) x + 1;
					current_y=(char) y + 1;
					display_dot(current_x,current_y);
					currentAngleX = currentAngleY = 0;
					if(threshold>0.1)
					{
						threshold=threshold-0.1;
					}
					sleep(1);
					elapsedTimeT2-=(2000000 + 400000*level);
				}
			}
			else
			{
				timer_on=1;
				centered_time=time(NULL);
			}
			
		}
		else
		{
			timer_on=0;
		}

		if (abs(currentAngleX) > threshold)
		{
			x = x+currentAngleX*0.002*level;
		}
		
		if(abs(currentAngleY) > threshold)
		{
			y = y+currentAngleY*0.002*level;
		}

		current_x = (char) x+1;
		current_y = (char) y+1;
		display_dot (current_x, current_y);
		elapsedTimeT2+=(Microsecs()-curTime);
		usleep(10000);
		iterationsT2++;
	}
	
}

int main()
{
	int fd;
	int i;
	long xGyroReading, yGyroReading, zGyroReading;
	long xAccReading, yAccReading, zAccReading;
	pthread_t thread;
	struct input_event ie;
	long curTime;
	gpio_initialize();
	fd2 = open("/dev/spidev1.0", O_RDWR);
	led_grid_initialize();
	
	if((fd = open(KEYFILE, O_RDONLY)) == -1) 
	{
		perror("opening device");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{	
			printf("\ncan't catch SIGINT\n");
 	}
	
	display_dest();
	display_fire();
	pthread_create (&thread, NULL, moveLed, NULL);

	while(1)
	{
		while(read(fd, &ie, sizeof(struct input_event))) 
		{
			curTime = Microsecs();
			if(ie.type == EV_ABS )
			{
				if(ie.code ==ABS_X)
				{
					xAccReading = ie.value;
				}

				if(ie.code ==ABS_Y)
				{
					yAccReading = ie.value;
				}

				if(ie.code ==ABS_Z)
				{
					zAccReading = ie.value;
				}

				if(ie.code ==ABS_RX)
				{
					xGyroReading = ie.value;
				}

				if(ie.code ==ABS_RY)
				{
					yGyroReading = ie.value;
				}

				if(ie.code ==ABS_RZ)
				{
					zGyroReading = ie.value;
				}

			}
			if ((ie.type == EV_SYN) && ((abs(xGyroReading)>10*131) || 
					(abs(yGyroReading)>10*131) || (abs(zGyroReading)>10*131)))
			{
				complementaryFilter(xGyroReading, yGyroReading, zGyroReading,
										xAccReading, yAccReading, zAccReading, &currentAngleX, &currentAngleY);
			}

			elapsedTimeT1+=(Microsecs()-curTime);
			iterationsT1++;
		}
	}
	return 0;
}
