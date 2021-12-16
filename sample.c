#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main(){
 //char buf[10];
int i=0;
for(i=0;i<10;i++)
{
 
 printf("%d : Hello\n",i);
 sleep(2);
}
 //buf[a]='\0';
 //write(1,buf,10);
 //printf("hello\n");
}
