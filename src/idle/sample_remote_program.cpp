#include<stdio.h>
#include<unistd.h>
int main(){
 //char buf[10];
 FILE *fp;
 fp=fopen("nano.txt","w");
 fprintf(fp,"hello\n");
 fclose(fp);
 //buf[a]='\0';
 //write(1,buf,10);
 //printf("hello\n");
}
