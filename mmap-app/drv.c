/* Test Code to write to a MMAPed memory space
* provided by a simple character driver
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <assert.h>


void main()
{

int fd = open("/dev/pci_mmap", O_RDONLY, 0);
assert(fd !=-1);

char *mem = mmap(NULL, 1024 * 4096, PROT_READ, MAP_SHARED, fd, 0);

assert(mem !=MAP_FAILED);

write(1, (char*)mem, 6);
munmap(mem, 1024 * 4096);
close (fd);
}


