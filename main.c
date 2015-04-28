#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
struct PN{
  int PrimeNumber;
  struct PN *n;
};
struct list{
  int Prime;
  struct list *next;
  int Recurence;
};

//liste de nombre premiers
struct PN *ListPN=NULL;

//liste des différents facteurs et leurs occurences
struct list *global=NULL;
void PopList(struct list **l){
  struct list *pop;
  pop=(*l)->next;
  free(*l);
  *l=pop;
}
void PushList(struct list **l,int a){
  struct list *push=malloc(sizeof(struct list));
  push->Prime=a;
  push->next=*l;
  *l=push;
}
void PrintList(struct list *l){
  while(l!=NULL){
    printf("%d   %d\n",l->Prime,l->Recurence);
    PopList(&l);
  }
}
int IsPrimeNumber(int a){
  int i;
  float v = sqrt((float) a);
  for (i=2;i<=v;i++){
    if(a % i == 0){
      return 0;
    }
  }
  return 1;
}

int NextPrimeNumber(int a){
  int i=1;
  int v=a+1;
  while (i<100000000){
    if (IsPrimeNumber(v)){
      return v;
    }
    else {
      v++;
    }
  }
  printf("erreur NextPrimeNumber");
  return 0; //à changer
}

struct list *Factorisation(int a){
  struct list *l=NULL;
  struct PN *parcours=ListPN;//node qui parcours la liste de nombre premiers
  int v;
  while (a!=1){
		v=parcours->PrimeNumber;
    	if (a % v==0){
     	 a= a/v;
      	PushList(&l,v);
      	// printf("%d \n",i);
     	 parcours=ListPN;
    	}
    	else {
    		if (parcours->n==NULL){//rajoute un PrimeNumber à la liste
    	  		struct PN *push=malloc(sizeof(struct PN));
    	  		push->PrimeNumber=NextPrimeNumber(v);
    	  		push->n=NULL;
    	  		parcours->n=push;
    	  	}
    	  	parcours=parcours->n;
      
   	}
  }
  return l;
  
}
void Remplissage( struct list *l){
  struct list *parcours = global;
  while (l!=NULL){
    if (parcours==NULL){
      PushList(&global,l->Prime);
      global->Recurence=1;
      PopList(&l);
      parcours=global;
    }
    else if (l->Prime==parcours->Prime){
      if(parcours->Recurence){
	parcours->Recurence++;
      }
	PopList(&l);
	parcours=global;
    }
    else{
      parcours=parcours->next;
    }
  }
}
void Producer(char *file){
  FILE* fichier = NULL;

    fichier = fopen(file, "r");
    int caractereActuel = 0;
    uint64_t var;
    struct list *l;
    if (fichier != NULL)
    {
      size_t fin=fread( &var , sizeof(var) , 1 , fichier );
      while (fin!=0){ //en fait un de trop
			var=be64toh(var);
			printf("%lld\n",var);
			l=Factorisation(var);
			Remplissage(l);
			
			fin=fread( &var , sizeof(var) , 1 , fichier );
      }
      PrintList(global);
      fclose(fichier);
    }
    else
    {
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier file1.txt\n");
    }
  
}
void initialisation(void){
	struct PN *P = malloc(sizeof(struct PN));
	P->PrimeNumber=2; 
	P->n=NULL;
	ListPN=P; 
}

int main ( int argc, char *argv[]){
	//struct PN *ListPN=(struct PN *)malloc(sizeof(struct PN));
	initialisation();
   int b=50; 
  /* printf("mmmmmh \n"); */
  /* if (IsPrimeNumber(b)){ */
  /*   printf("ouiiii \n"); */
  /* } */
  /* else{ */
  /*   printf("nonnnnn \n"); */
  /* } */
  /*  printf("%d \n",NextPrimeNumber(b)); */
  
  int i=0;
  int c=2;
  for (i=0;i<200;i++){
  	//printf("%d\n",c);
  	c=NextPrimeNumber(c);
  }
  
  //struct list *l=Factorisation(b);
   printf("ok\n"); 
   //Remplissage(l); 
   //PrintList(l); 
   //PrintList(global);
  Producer("file1.txt");
}
