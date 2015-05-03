#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <semaphore.h>
#define NB_phtread_mutex_t  6
#define NB_sem_t  5
#define maxthread 2
int m=0;
int n=0;
int ProdCount=0;
int ConsCount=0;
int RempCount=0;
int Cons2Count=0;
int T[maxthread];
struct list *T2[maxthread];
int flag=0;
int flag2=0;
int flag3=0;

struct PN{
  int PrimeNumber;
  struct PN *n;
};
struct list{
  int Prime;
  struct list *next;
  int Recurence;
};

sem_t semaphore [NB_sem_t] ;
pthread_mutex_t mutex [NB_phtread_mutex_t] ;


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
    	  		pthread_mutex_lock(&mutex[3]); ///!!!!! probleme mutex
    	  		push->PrimeNumber=NextPrimeNumber(v);
    	  		push->n=NULL;
    	  		parcours->n=push;
    	  		pthread_mutex_unlock(&mutex[3]);
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
	parcours->Recurence=0;
      }
	PopList(&l);
	parcours=global;
    }
    else{
      parcours=parcours->next;
    }
  }
}
void *producer(void *file){
  FILE* fichier = NULL;
		//printf("%c\n",*((char *) file));
    fichier = fopen((char *) file, "r");
    uint64_t var;
    struct list *l;
    //printf("1");
    if (fichier != NULL)
    {
    	//printf("2");
    	pthread_mutex_lock(&mutex[1]);
      size_t fin=fread( &var , sizeof(var) , 1 , fichier );
      pthread_mutex_unlock(&mutex[1]);
      while (fin!=0){ 
      	//printf("3");
      	sem_wait(&semaphore[1]);
      	pthread_mutex_lock(&mutex[1]);
      	ProdCount++;
			T[m]=be64toh(var);
			m++;
			//printf(" m:%d ",m);
			fin=fread( &var , sizeof(var) , 1 , fichier );
		//	if(fin==0){
		//		flag++;
		//	}
			pthread_mutex_unlock(&mutex[1]);
			sem_post(&semaphore[2]);
      }
      //PrintList(global);
      printf("bam\n");
      //flag++;
      fclose(fichier);
      pthread_mutex_lock(&mutex[1]);
      flag++;
      pthread_mutex_unlock(&mutex[1]);
      printf("flag:%d\n",flag);
    }
    else
    {
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier file1.txt\n");
    } 
    return NULL;
}
void *consummer(void *p){
	struct list *l=NULL;
	int g;
	int flag3=0;
	char *t= (char*) p;
	int *value;
	while(1){
		printf(" threada:%c ",*t);
		pthread_mutex_lock(&mutex[1]);
		sem_getvalue(semaphore[2], value);
		
		if (value==0 && flag==4){
			pthread_mutex_unlock(&mutex[1]);
			printf(" threadc:%c ",*t);
			break;
		}
		pthread_mutex_unlock(&mutex[1]);
		printf(" threadd:%c ",*t);
		printf(" m:%d ",m);
		sem_wait(&semaphore[2]);
		//pthread_mutex_lock(&mutex[0]);
		pthread_mutex_lock(&mutex[1]);
		if (m==0 && flag==4){
		printf(" threadg:%c ",*t);
			break;
		}
		ConsCount++;
		
		m--;
		//printf(" m:%d ",m);
		g=T[m];
		//Remplissage(l);
		//if (m==0 && flag==4){
			//flag3++;
			//printf("flag3:%d\n",flag3);
		//}
		printf(" threadh:%c ",*t);
		pthread_mutex_unlock(&mutex[1]);
		//pthread_mutex_unlock(&mutex[0]);
		sem_post(&semaphore[1]);
		
		l=Factorisation(g);
		//printf("1");
		printf(" threadi:%c ",*t);
		printf(" n:%d ",n);
		sem_wait(&semaphore[3]);
		
		//pthread_mutex_lock(&mutex[0]);
		printf(" threadk:%c ",*t);
		pthread_mutex_lock(&mutex[2]);
		Cons2Count++;
		//printf("2");
		T2[n]=l;
		//printf("3");
		n++;
		//printf(" n:%d ",n);
		//printf("flag:%d\n",flag2);
		printf(" threadj:%c ",*t);
		pthread_mutex_unlock(&mutex[2]);
		//pthread_mutex_unlock(&mutex[0]);
		sem_post(&semaphore[4]);
	}
	pthread_mutex_lock(&mutex[2]);
	flag2++;
	printf(" flag2:%d ",flag2);
	pthread_mutex_unlock(&mutex[2]);
	printf("bim\n");
	printf("n: %d\n",n);
	printf("ProdCount: %d\n",ProdCount);
  	printf("RempCount: %d\n",RempCount);
  	printf("ConsCount: %d\n",ConsCount);
	return NULL;
}
void *remplisseur(void *p){
	struct list *l;
	while(1){
	pthread_mutex_lock(&mutex[2]);
		if (n==0 && flag2==maxthread){
			printf("break remp");
			break;
		}
	pthread_mutex_unlock(&mutex[2]);
	printf("4");
		sem_wait(&semaphore[4]);
		printf("5");
		pthread_mutex_lock(&mutex[2]);
		if (n==0 && flag2==maxthread){
			printf("break remp");
			break;
		}
		RempCount++;
		printf("6");
		//printf(" n:%d ",n);
		n--;
		//printf(" n:%d ",n);
		Remplissage(T2[n]);
		//printf("7");
		pthread_mutex_unlock(&mutex[2]);
		sem_post(&semaphore[3]);
		printf(" ");
		//printf(" n:%d ",n);
		printf(" flag2:%d ",flag2);
	}
	PrintList(global);
	printf("boum\n");
	
	return NULL;
}
void initialisation(void){
	struct PN *P = malloc(sizeof(struct PN));
	P->PrimeNumber=2; 
	P->n=NULL;
	ListPN=P; 
}
void freePN(void){
	struct PN *pop;
	while(ListPN!=NULL){
		
 		pop=ListPN->n;
  		free(ListPN);
  		ListPN=pop;
	}
}

int main ( int argc, char *argv[]){
	//struct PN *ListPN=(struct PN *)malloc(sizeof(struct PN));
	initialisation();
	int b=0;
	int i=0;  
  	sem_init (&semaphore[1], 0, maxthread);
	sem_init (&semaphore[2], 0, 0);
	sem_init (&semaphore[3], 0, maxthread);
	sem_init (&semaphore[4], 0, 0);
	sem_init (&semaphore[5], 0, maxthread);
	pthread_mutex_init(&mutex[1],NULL);
	pthread_mutex_init(&mutex[2],NULL);
	pthread_mutex_init(&mutex[3],NULL);
	pthread_mutex_init(&mutex[0],NULL);
  	pthread_t prod , prod2 , prod3,prod4, remp;
  	pthread_t cons[maxthread];
  //struct list *l=Factorisation(b);
   //printf("ok\n"); 
   //Remplissage(l); 
   //PrintList(l); 
   //PrintList(global);
  pthread_create(&prod,NULL,&producer,(void *) "file1.txt" );
   pthread_create(&prod2,NULL,&producer,(void *) "file2.txt" );
   pthread_create(&prod3,NULL,&producer,(void *) "file3.txt" );
   pthread_create(&prod4,NULL,&producer,(void *) "file4.txt" );
   pthread_create(&cons[0],NULL,&consummer,(void *) "0");
   pthread_create(&cons[1],NULL,&consummer,(void *) "1");
  //Producer("file1.txt");
  //for (i=0;i<maxthread;i++){
  	//pthread_create(&cons[i],NULL,&consummer,NULL);
  //}
  	
  	pthread_create(&remp,NULL,&remplisseur,NULL);
  	//remplisseur((void *) &n);
  	//PrintList(global);
  	pthread_join (prod, NULL);
  	pthread_join (prod2, NULL);
  	pthread_join (prod3, NULL);
  	pthread_join (prod4, NULL);
  	
  	//pthread_join (cons[0], NULL);
 	for (i=0;i<maxthread;i++){
 		pthread_join (cons[i], NULL);
 	}
	pthread_join (remp, NULL);
  	freePN();
  	printf("ProdCount: %d\n",ProdCount);
  	printf("RempCount: %d\n",RempCount);
  	printf("ConsCount: %d\n",ConsCount);
  	printf("Cons2Count: %d\n",Cons2Count);
  
}
