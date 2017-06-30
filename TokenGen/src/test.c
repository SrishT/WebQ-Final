#include <stdio.h>
void main(){
	FILE *fptr;
	fptr = fopen("IdStore","r");
    if(fptr != NULL){
    	printf("In read id -- if\n");
        char c = fgetc(fptr);
        id = atoi(&c);
        // fscanf(fptr,"%d",&id);
    	fclose(fptr); 
    }
    else{
    	printf("In read id -- else\n");
    }
}