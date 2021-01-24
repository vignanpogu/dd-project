
/* User Application to test the linux character Device Driver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int8_t write_buf[1024];
int8_t read_buf[1024];

int main()
{

	int fd;
	char op;
	printf(" Testing the character device driver..\n");
	
	fd = open("/sys/kernel/sysfs_drv/chr_value", O_RDWR);
	if (fd <0) {
		printf("Cannot open device file .\n");	
		return 0;
	}

	while(1)
	{

	printf(" Please enter your operation to do in character device driver..\n");
	printf(" 1. Write	\n");
	printf(" 2. Read	\n");
	printf(" 3. Exit	\n");
	scanf(" %c", &op);
	printf(" You choose the operation = %c\n", op);

	switch(op) {

		case '1':
				printf(" Enter the data which you want to write..\n");
				scanf(" %[^\t\n]s",  write_buf);
				printf(" Data is written..\n");
				write(fd, write_buf, strlen(write_buf)+1);
				break;
		case '2':
				printf(" Reading the data..\n");
				read(fd, read_buf, 1024);
				printf("Data = %s\n", read_buf);
				break;
		case '3':
				close(fd);
				exit(1);
				break;
		default :
				printf(" Please enter the valid option..\n");
				break;
		}
	}

	close(fd);
}
