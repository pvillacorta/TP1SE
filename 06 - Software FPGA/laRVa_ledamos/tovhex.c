#include <stdlib.h>
#include <stdio.h>


#define ROMBASE	(0) 		//0xE000 & 0x1F00)
#define ROMSIZE (0x2000*2)

unsigned char mem[ROMSIZE];

main(int argc, char **argv)
{
	int i,n;
	FILE *fp;

	if ((fp=fopen(argv[1],"rb"))==NULL) exit(1);
	bzero(mem,ROMSIZE);
	n=fread(mem,1,ROMSIZE,fp);
	fclose(fp);


	if ((fp=fopen(argv[2],"wb"))==NULL) exit(1);
	//fprintf(fp,"@%04X\n",ROMBASE);
	for (i=0;i<ROMSIZE;i+=4) fprintf(fp,"%08X\n",mem[i]+(mem[i+1]<<8)+(mem[i+2]<<16)+(mem[i+3]<<24));
	fclose(fp);
	return 0;
}

