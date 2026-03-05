#include<stdio.h>
#include<stdlib.h>

int main(int argc, char*argv[]){
  if(argc!=3){
    printf("debe haber num_1 num_2\n",argv[0]);
    return(1);
  }
  else
    {
      int num_1=atoi(argv[1];
      int num_2=atoi(argv[2];
      menor(num_1,num_2);
    }
  return 0;
  
}


void menor(int ent_1,int ent_2)
{
  char resultado[50];
  char mayor[]=" es mayor que ";
  char menor[]=" es menor que ";
  char igual[]=" es igual que";
  if(ent_1>ent_2)
    {
      snprintf(resultado,sizeof(resultado),"%d%s%d",ent_1,mayor,ent_2);
      printf("%s\n",resultado);
    }
   if(ent_1<ent_2)
    {
      snprintf(resultado,sizeof(resultado),"%d%s%d",ent_1,menor,ent_2);
      printf("%s\n",resultado);
    }
   if(ent_1==ent_2)
    {
      snprintf(resultado,sizeof(resultado),"%d%s%d",ent_1,igual,ent_2);
      printf("%s\n",resultado);
    }
  

}
