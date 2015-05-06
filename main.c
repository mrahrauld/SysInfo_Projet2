#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <semaphore.h>
#include <sys/time.h>
#define NB_phtread_mutex_t  6
#define NB_sem_t  5
#define MAXTHREAD "-maxthreads"
#define STDIN "-stdin"
#define prefix_URL "http://"
#define prefix_URL_LENGTH 7
int prodcount=0; // permet de calculer le nombre de producteurs crees
int maxthread=4; //initialisé à 4 de maniere arbitraire, modifiee dans initialisation
int Buffer1_Count=0; 
int Buffer2_Count=0;
int *Buffer1;
char *Arg;
int breaker=0;
int value;
struct list **Buffer2;
int flag=0;
int flag2=0;
int flag3=0;
int IsStdinFinish=0;
pthread_t prod[20];

int max1=0;
int min1=0;
int max2=0;
int min2=0;

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
void clean(const char *buffer, FILE *fp)//http://openclassrooms.com/courses/utiliser-les-bonnes-fonctions-d-entree
{
    char *p = strchr(buffer,'\n');
    if (p != NULL)
        *p = 0;
    else
    {
        int c;
        while ((c = fgetc(fp)) != '\n' && c != EOF);
    }
}
int IsPrimeNumber(int a){
  int i=2;
  float v = sqrt((float) a);
  if(a % i == 0){
      return 0;
  }
  i++;
  while (i<=v){
    if(a % i == 0){
      return 0;
    }
    i=i+1;
  }
  return 1;
}

int NextPrimeNumber(int a){
  int v=a+1;
  if (v % 2 ==0){
  	v++;
  }
  while (v<100000000){
    if (IsPrimeNumber(v)){
      return v;
    }
    else {
      v=v+1;
    }
  }
  printf("erreur NextPrimeNumber");
  return 0; //à changer
}
void initialisation(void){
	int *Buff= malloc(sizeof(int)*maxthread);
	Buffer1=Buff;
	char *A= malloc (sizeof(char)*maxthread);
	Arg=A;
	struct list **Buff2= malloc (sizeof(struct list*)*maxthread);
	Buffer2=Buff2;
	struct PN *P = malloc(sizeof(struct PN));
	P->PrimeNumber=2; 
	P->n=NULL;
	ListPN=P;
	struct list *glob=malloc(sizeof(struct list));
	printf("3");
	glob->Prime=2;
	glob->Recurence=2;
	glob->next=NULL;
	global=glob;
	
	sem_init (&semaphore[1], 0, maxthread);
	sem_init (&semaphore[2], 0, 0);
	sem_init (&semaphore[3], 0, maxthread);
	sem_init (&semaphore[4], 0, 0);
	sem_init (&semaphore[5], 0, maxthread);
	pthread_mutex_init(&mutex[1],NULL);
	pthread_mutex_init(&mutex[2],NULL);
	pthread_mutex_init(&mutex[3],NULL);
	pthread_mutex_init(&mutex[0],NULL);
}

void define_maxthread(int argc, char *argv[]){
	int i=0;  
	for (i=1; i < argc-1; i++)
		{
			if (strcmp(argv[i], MAXTHREAD) == 0)
			{
				maxthread = atoi(argv[i+1]);
				i++;
	  		}
	}
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
     	 parcours=ListPN;
    	}
    	else { 
    		if (parcours->n==NULL){//rajoute un PrimeNumber à la liste		
    	  		struct PN *push=malloc(sizeof(struct PN));
    	  		pthread_mutex_lock(&mutex[3]); 
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
  	if (l->Prime==parcours->Prime){
   	   if(parcours->Recurence){
				parcours->Recurence--;
      	}
		PopList(&l);
		parcours=global;
   }	
   else if (parcours->next==NULL){
    	struct list *push= malloc (sizeof(struct list));
    	push->Recurence=1;
    	push->Prime=l->Prime;
    	push->next=NULL;
    	parcours->next= push;
      PopList(&l);
      parcours=global;
    }
    else{
      parcours=parcours->next;
    }
  }
}

void *producer(void *file){
	
 	 FILE* fichier = (FILE *) file;
     pthread_mutex_lock(&mutex[0]);
		pthread_mutex_unlock(&mutex[0]);
    uint64_t var;
    struct list *l;

    if (fichier != NULL)
    {

    	pthread_mutex_lock(&mutex[1]);
      size_t fin=fread( &var , sizeof(var) , 1 , fichier );
      pthread_mutex_unlock(&mutex[1]);
      while (fin!=0){ 
      	sem_wait(&semaphore[1]);
      	pthread_mutex_lock(&mutex[1]);
			Buffer1[Buffer1_Count]=be64toh(var);
			Buffer1_Count++;
			fin=fread( &var , sizeof(var) , 1 , fichier );
			pthread_mutex_unlock(&mutex[1]);
			sem_post(&semaphore[2]);
      }
      printf("bam\n");
      fclose(fichier);
      pthread_mutex_lock(&mutex[1]);
      flag++;
      if (flag==prodcount) {
       sem_post(&semaphore[2]);
      }
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
	while(1){
		pthread_mutex_lock(&mutex[1]);
		if (Buffer1_Count==0 && flag==prodcount){
			breaker=1;
		   Buffer1_Count=maxthread;
		   sem_post(&semaphore[3]);
		   sem_post(&semaphore[2]);
			pthread_mutex_unlock(&mutex[1]);
			
			break;
		}
		pthread_mutex_unlock(&mutex[1]);
		sem_wait(&semaphore[2]);
		pthread_mutex_lock(&mutex[1]);
		if (breaker || Buffer1_Count==0 && flag==prodcount){
		   	sem_post(&semaphore[2]);
		   pthread_mutex_unlock(&mutex[1]);
			break;
		}
		if (Buffer1_Count==maxthread){
		max1++;
		}
		Buffer1_Count--;
		if (Buffer1_Count==0){
		min1++;
		}
		g=Buffer1[Buffer1_Count];
		pthread_mutex_unlock(&mutex[1]);
		sem_post(&semaphore[1]);
		
		l=Factorisation(g);

		sem_wait(&semaphore[3]);
		pthread_mutex_lock(&mutex[2]);

		Buffer2[Buffer2_Count]=l;
		if (Buffer2_Count==0){
		min2++;
		}
		Buffer2_Count++;
		if (Buffer2_Count==maxthread){
		max2++;
		}
		pthread_mutex_unlock(&mutex[2]);
		sem_post(&semaphore[4]);
	}
	pthread_mutex_lock(&mutex[2]);
	flag2++;

	if (flag2==maxthread){
		sem_post(&semaphore[4]);
	}
	pthread_mutex_unlock(&mutex[2]);
	
	printf("bim\n");
	return NULL;
}
void *remplisseur(void *p){
	struct list *l;
	while(1){
	pthread_mutex_lock(&mutex[2]);
		if (Buffer2_Count==0 && flag2==maxthread){
			printf("break remp");
			break;
		}
	pthread_mutex_unlock(&mutex[2]);
		sem_wait(&semaphore[4]);

		pthread_mutex_lock(&mutex[2]);
		if (Buffer2_Count==0 && flag2==maxthread){
			printf("break remp");
			break;
		}

		Buffer2_Count--;
		Remplissage(Buffer2[Buffer2_Count]);

		pthread_mutex_unlock(&mutex[2]);
		sem_post(&semaphore[3]);
	}
	printf("boum\n");
	
	return NULL;
}

void fin(void){
	struct list *parcours = global;
	while(parcours!=NULL){
		if(parcours->Recurence==1){
			printf("le seul nombre premier utilisé une fois est:%d\n",parcours->Prime);
		}
		parcours=parcours->next;
	}
}


void freePN(void){
	struct PN *pop;
	while(ListPN!=NULL){
		//printf(" %d ",ListPN->PrimeNumber);
 		pop=ListPN->n;
  		free(ListPN);
  		ListPN=pop;
	}
}
void Launch_producer(int argc, char *argv[]){
	int c;
	FILE* fichier[argc];
	for (c=1; c < argc; c++){
		if(strcmp(argv[c],MAXTHREAD)==0){
			c++;
		}
		else if(strcmp(argv[c],prefix_URL)==0){
		//URL ET MACHIN CHOSE
		}
		else if(strcmp(argv[c],STDIN)!=0){
			fichier[c]  = fopen(argv[c], "r");
			if (fichier[c]!= NULL){
			prodcount++;
			pthread_create(&prod[prodcount],NULL,&producer,(void *) fichier[c] );
			}
		}
	}
	
}

int main ( int argc, char *argv[]){
	struct timeval tbegin,tend;
    double texec=0.;
	gettimeofday(&tbegin,NULL);
	
	define_maxthread(argc , argv);
	
	initialisation();
	pthread_mutex_lock(&mutex[0]);
	Launch_producer(argc , argv);
	printf("prodcount:%d\n",prodcount);
	char BUFF[20];
	int i;
	pthread_t cons[maxthread];
	pthread_t remp;
	for (i=0;i<maxthread;i++){
  	pthread_create(&cons[i],NULL,&consummer,(void *) "1");
  	}
  	pthread_create(&remp,NULL,&remplisseur,NULL);
  	
	for (i=1; i < argc; i++){
		if(strcmp(argv[i],STDIN)==0){
			while (1){
				printf("Inserez un nombre ou /Exit : ");
				fgets(BUFF, sizeof(BUFF), stdin);
				clean(BUFF, stdin);
				if(strcmp("/Exit",BUFF)==0)
				{
					break;
				}
				int var= NULL;
				var=atoi(BUFF);
				if(var){
				printf("vous avez inseré: %d\n",var);
				sem_wait(&semaphore[1]);  
				pthread_mutex_lock(&mutex[1]);
				Buffer1[Buffer1_Count]=var; 
				Buffer1_Count++; 
				pthread_mutex_unlock(&mutex[1]); 
				sem_post(&semaphore[2]);
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex[0]);
	int b=0;  
  
	//for (i=0;i<prodcount;i++){
 		//pthread_join (prod[i], NULL);
 	//}
  	//pthread_join (prod, NULL);
  	//pthread_join (prod2, NULL);
  //	pthread_join (prod3, NULL);
  //	pthread_join (prod4, NULL);
  printf("2\n");
 	for (i=0;i<maxthread;i++){
 		pthread_join (cons[i], NULL);
 	}
	pthread_join (remp, NULL);

  freePN();
  	
  	fin();
  	printf("min1 %d\n", min1);
  	printf("max1 %d\n", max1);
  	printf("min2 %d\n", min2);
  	printf("max2 %d\n", max2);
  	gettimeofday(&tend,NULL);
  	texec=((double)(1000*(tend.tv_sec-tbegin.tv_sec)+((tend.tv_usec-tbegin.tv_usec)/1000)))/1000.;
  	
  	printf(" %f\n",texec);
}
