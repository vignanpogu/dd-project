
/* User Application to test the linux character Device Driver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define WR_IOCTL _IOW('a','a',int32_t*)
#define RD_IOCTL _IOR('a','b',int32_t*)

int main()
{

	int fd;
	int32_t num, value;
	printf(" Testing the character device driver..\n");
	
	fd = open("/dev/chr_device", O_RDWR);
	if (fd <0) {
		printf("Cannot open device file .\n");	
		return 0;
	}

	printf("Enter the value to write..\n");
	scanf("%d", &num);
	printf("Entering the value to the Device Driver..\n");
	ioctl(fd, WR_IOCTL, (int32_t*) &num);

	printf("Reading the value from the driver..\n");
	ioctl(fd, RD_IOCTL, (int32_t*) &value);
	printf("value is %d\n", value);

	printf("Close the Driver ..\n");
	close(fd);
}
