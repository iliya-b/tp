#include "stdio.h"
#include "string.h"


enum Script {
	DEFAULT,
	NEWLINES,
	FFLUSH,
	NEWLINES_FFLUSH
};

int main(int argc, char* argv[]){
	char str_out[15] = "Hello, world!";
	char str_err[10] = "Error!";
	enum Script script = DEFAULT;

	if(argc > 1) {
                if(!strcmp(argv[1], "--newlines")) script = NEWLINES; 
                else if(!strcmp(argv[1], "--newlines-fflush")) script = NEWLINES_FFLUSH;
                else if(!strcmp(argv[1], "--fflush")) script = FFLUSH;
		else {
                        puts("Allowed arguments: --newlines --newlines-flush --fflush");
			return 1;
		}
	}
	if(script == NEWLINES || script == NEWLINES_FFLUSH){
                strcpy(str_out, "Hello, world!\n");
        	strcpy(str_err, "Error!\n");
	}

	for (int i = 0; i < 5; i++) {
		fputs(str_out, stdout);
		fputs(str_err, stderr);
                if(script == FFLUSH || script == NEWLINES_FFLUSH) {
			fflush(stdout);
                        fflush(stderr);
		}
	}

	return 0;
}
