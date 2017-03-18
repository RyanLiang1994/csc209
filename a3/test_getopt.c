#include <unistd.h>                                                                                            
#include <getopt.h>  
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
int main(int argc, char * const argv[]) {  
	int opt; 
	while ((opt = getopt(argc, argv, "nb::o::t")) != -1) {  
		switch (opt) {
			case 'n':
				printf("opt = %c, optarg = %s, optind = %d, argv[%d] = %s\n",
				            opt, optarg, optind, optind, argv[optind]);
			case 'b':
				optarg = NULL;
				printf("opt = %c, optarg = %s, optind = %d, argv[%d] = %s\n",
				            opt, optarg, optind, optind, argv[optind]);
			case 't':
				printf("opt = %c, optarg = %s, optind = %d, argv[%d] = %s\n",
				            opt, optarg, optind, optind, argv[optind]);
			case 'o':
				printf("opt = %c, optarg = %s, optind = %d, argv[%d] = %s\n",
				            opt, optarg, optind, optind, argv[optind]);
		}
	}
	  
	return 0;  
}  
