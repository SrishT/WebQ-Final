#include <stdio.h>
#include <stdlib.h>
void main(){
	FILE *fptr;
    int id;
	fptr = fopen("IdStore","r");
    if(fptr != NULL){
    	printf("In read id -- if\n");
        char c = fgetc(fptr);
        id = atoi(&c);
        printf("id = %d\n",id);
        // fscanf(fptr,"%d",&id);
    	fclose(fptr); 
    }
    else{
    	printf("In read id -- else\n");
    }
}