#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include <stdlib.h>
#include <stdbool.h>


int main(int argc, char* argv[]){
    size_t memory = 0;
    bool read_option = false;
    bool write_option = false;

    int rez = 0;
	while ( (rez = getopt(argc, argv, "m:rw")) != -1){
		switch (rez) {
            case 'r': read_option = true; break;
            case 'w': write_option = true; break;
            case 'm': memory = atoi(optarg); break;
            case '?': printf("Incorrect options\n"); return -1;
		}
	}
    if(!(read_option ^ write_option)){
        printf("You must specify either -r or -w option\n");
        return -1;
    }
    char *p = malloc(memory);
    if (p == NULL) {
        fprintf(stderr, "malloc failed\n");
        return -1;
    }
    int step = 1024 * 4;

    char current_value = '1';
    unsigned int i = 0;
    while((i+1) * step < memory){
        if(read_option){
            current_value = p[i * step];
        }
        if(write_option){
            for(int j = 0; j < 500; j += 50){
                p[i * step + j] = current_value;
            }
        }

        if(i % 4000) {
            usleep (1000 * 400);
        }
        i++;
    }
	return 0;
}
