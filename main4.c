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
#include <string.h>
//#include<curl/curl.h>
#define NB_phtread_mutex_t  6
#define NB_sem_t  5
#define maxthreads "-maxthreads"
//#define maxthread 4
#define STDIN "-stdin"
#define STDIN_filename "stdin"
#define prefix_URL "http://"
//#define prefix_URL_long 5
int maxthread;
int i;
int m=0;
int n=0;
int ProdCount=0;
int ConsCount=0;
int RempCount=0;
int Cons2Count=0;
int T[];
char Arg[];
int breaker=0;
int value;
struct list *T2[];
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
    		//pthread_mutex_lock(&mutex[3]); 
    		if (parcours->n==NULL){//rajoute un PrimeNumber à la liste		
    	  		struct PN *push=malloc(sizeof(struct PN));
    	  		pthread_mutex_lock(&mutex[3]); 
    	  		push->PrimeNumber=NextPrimeNumber(v);
    	  		push->n=NULL;
    	  		parcours->n=push;
    	  		pthread_mutex_unlock(&mutex[3]);
    	  	}
    	  	//pthread_mutex_unlock(&mutex[3]);
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
      //PushList(&global,l->Prime);
      //global->Recurence=1;
      PopList(&l);
      parcours=global;
    }
    else{
    	//precedent=parcours;
      parcours=parcours->next;
    }
  }
}
void *producer(void *fichier){
  //FILE* fichier = NULL;
		//printf("%c\n",*((char *) file));
    //fichier = fopen((char *) file, "r");
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
      	//ProdCount++;
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
      if (flag==4) {
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
		//printf(" threada:%c ",*t);
		pthread_mutex_lock(&mutex[1]);
		if (m==0 && flag==4){
			breaker=1;
		   m=maxthread;
		   sem_post(&semaphore[3]);
		   sem_post(&semaphore[2]);
		   //sem_post(&semaphore[4]);
			pthread_mutex_unlock(&mutex[1]);
			//printf(" threadc:%c ",*t);
			
			break;
		}
		pthread_mutex_unlock(&mutex[1]);
		//printf(" threadd:%c ",*t);
		//printf(" m:%d ",m);
		sem_wait(&semaphore[2]);
		pthread_mutex_lock(&mutex[1]);
		if (breaker || m==0 && flag==4){
			//sem_post(&semaphore[3]);
		   sem_post(&semaphore[2]);
		   pthread_mutex_unlock(&mutex[1]);
			//printf(" threadg:%c ",*t);
			break;
		}
		//ConsCount++;
		
		m--;
		//printf(" m:%d ",m);
		g=T[m];
		//Remplissage(l);
		//if (m==0 && flag==4){
			//flag3++;
			//printf("flag3:%d\n",flag3);
		//}
		//printf(" threadh:%c ",*t);
		pthread_mutex_unlock(&mutex[1]);
		sem_post(&semaphore[1]);
		
		l=Factorisation(g);
		//printf("1");
		//printf(" threadi:%c ",*t);
		//printf(" n:%d ",n);
		sem_wait(&semaphore[3]);
		
		//printf(" threadk:%c ",*t);
		pthread_mutex_lock(&mutex[2]);
		//Cons2Count++;
		//printf("2");
		T2[n]=l;
		//printf("3");
		n++;
		//printf(" n:%d ",n);
		//printf("flag:%d\n",flag2);
		//printf(" threadj:%c ",*t);
		pthread_mutex_unlock(&mutex[2]);
		//pthread_mutex_unlock(&mutex[0]);
		sem_post(&semaphore[4]);
	}
	pthread_mutex_lock(&mutex[2]);
	flag2++;
	//printf(" flag2:%d ",flag2);
	//sem_getvalue(&semaphore[3], &value);
	//printf("sem3: %d\n",value);
	//sem_getvalue(&semaphore[4], &value);
	//printf("sem4: %d\n",value);
	if (flag2==maxthread){
		sem_post(&semaphore[4]);
	}
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
	//printf("4");
		sem_wait(&semaphore[4]);
		//printf("5");
		pthread_mutex_lock(&mutex[2]);
		if (n==0 && flag2==maxthread){
			printf("break remp");
			break;
		}
		//RempCount++;
		//printf("6");
		//printf(" n:%d ",n);
		n--;
		//printf(" n:%d ",n);
		Remplissage(T2[n]);
		//printf("7");
		pthread_mutex_unlock(&mutex[2]);
		sem_post(&semaphore[3]);
		//printf(" ");
		//printf(" %d ",RempCount);
		//printf(" flag2:%d ",flag2);
	}
	//PrintList(global);
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

int main ( int argc, char *argv[]){
	
	  FILE *fichier = NULL;
	  //URL_FILE *fichier_url = NULL;
	  	
		//bool *Act_producer = malloc(argc,sizeof(bool));
		
		//bool s_stdin = false;
		int nb_threads = 0 ;
		char BUFF;
		
		//if(Act_producer == NULL)
		//{
			//exit(EXIT_FAILURE);
		//}
		
		//char stdin_filename[] = STDIN_FILENAME;
		
	 for (i=1; i < argc-1; i++)
		{
			if (strcmp(argv[i], maxthreads) == 0)
			{
				maxthread = atoi(argv[i+1]);
				i++;
	  	}
	  	/*
			else if(strcmp(argv[i],STDIN)==0)
			{
			
				printf("Tapez '/Exit' pour stopper la boucle");
				while (true)
				{
					fgets(BUFF, sizeof(BUFF), stdin);
					clean(BUFF, stdin);
					
					sem_wait(&semaphore[1]); 
					pthread_mutex_lock(&mutex[1]); 
					T[m]=var; 
					m++; 
					pthread_mutex_unlock(&mutex[1]); 
					sem_post(&semaphore[2]);
				
					if(strcmp("/Exit",BUFF))
					{
						break;
					}
				}	

				//s_stdin = true ;
				if (pthread_create(&producer[i],NULL,&producer,NULL))
				{
					exit(EXIT_FAILURE);
				}
				Act_producer[i] = true ;
				nb_threads ++;
			}
			
			else if(strcmp(argv[i],prefix_URL,strlen("http://"))==0)
			{
				fichier_url = url_fopen(argv[i],"r");
				if (fichier_url = NULL)
				{
					printf("Imposssible d'ouvrir le fichier");
				}
				
				
				 char *url = (char *) param;
         int pipefd[2];
				if (pipe(pipefd)) 
				{
					perror("pipe");
					exit(EXIT_FAILURE);
				}
				write_from_url_to_fd(url, pipefd[1]);
	      
				
				else
				{
					if (pthread_create(&producer[i],NULL,&producer,argv[i]))
					{
						exit(EXIT_FAILURE);
					}
					Act_producer[i] = true ;
					nb_threads ++;
				}
			}
			else
			{
			fichier  = fopen(argv[i], "r");
				if (fichier == NULL)
					{
						printf("Impossible d'ouvrir le fichier");
					}
					else
					{
					if (pthread_create(&producer[i],NULL,&producer,(void*) *fichier))
						{
							exit(EXIT_FAILURE);
						}
					}
				Act_producer[i] = true ;
				nb_threads ++;
				}
		*/
				
			}

	
	struct timeval tbegin,tend;
    double texec=0.;
	gettimeofday(&tbegin,NULL);

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
   pthread_create(&prod4,NULL,&producer,(void *) "4k" );
  // pthread_create(&cons[0],NULL,&consummer,(void *) "0");
   //pthread_create(&cons[1],NULL,&consummer,(void *) "1");
   //pthread_create(&cons[2],NULL,&consummer,(void *) "2");
  //Producer("file1.txt");
  for (i=0;i<maxthread;i++){
  	pthread_create(&cons[i],NULL,&consummer,(void *) "1");
  }
  	
  	pthread_create(&remp,NULL,&remplisseur,NULL);
  	//remplisseur((void *) &n);
  	
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
  	fin();
  	gettimeofday(&tend,NULL);
  	texec=((double)(1000*(tend.tv_sec-tbegin.tv_sec)+((tend.tv_usec-tbegin.tv_usec)/1000)))/1000.;
  	
  	printf(" %f\n",texec);
}
