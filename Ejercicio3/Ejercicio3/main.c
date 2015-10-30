//
//  main.c
//  Ejercicio3
//
//  Created by Josué Carlos García Puig on 29/10/15.
//  Copyright © 2015 Josue Garcia. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define N 4
#define TRANSFORMERS 7

pthread_mutex_t * niveles;
pthread_cond_t * sube_t;

int * pesos;
int * pesosActuales;

void * procesa_pedido(void *);

int main(int argc, const char * argv[]) {
    
    srand((int)time(NULL));
    
    pthread_t * robots = (pthread_t *) malloc(sizeof(pthread_t) * TRANSFORMERS);
    niveles = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * N);
    sube_t = (pthread_cond_t *) malloc(sizeof(pthread_cond_t) * N);
    pesos = (int *) malloc(sizeof(int) * N);
    pesosActuales = (int *) malloc(sizeof(int) * N);
    
    int i = 0;
    
    for(; i<N; ++i)
    {
        *(pesos+i) = rand() % (100-60) + 60;
        *(pesosActuales+i) = 0;
        printf("Peso del nivel %d es: %d\n",i,*(pesos+i));
    }
    
    for (i=0;i<N;++i){
        pthread_mutex_init(niveles+i,NULL);
        pthread_cond_init(sube_t+i,NULL);
    }
    
    pthread_t * aux;
    int indice = 0;
    
    for(aux = robots; aux<(robots+TRANSFORMERS); ++aux)
    {
        pthread_create(aux, NULL, procesa_pedido, (void *) ++indice);
    }
    
    /* Adjuntar los hilos */
    for (aux = robots; aux < (robots+TRANSFORMERS); ++aux) {
        pthread_join(*aux, NULL);
    }
    
    free(robots);
    free(niveles);
    free(pesos);
    
    return 0;
}

void * procesa_pedido(void * arg)
{
    int id = (int) arg;
    
    int peso = rand() % 50 + 10;
    int nivel = 0;
    printf("Soy el robot %d y mi peso es %d\n",id,peso);
    
    while (1)
    {
        int nivelPeso = *(pesos+nivel);
        pthread_mutex_lock((niveles+nivel));
        if(nivel == N)
            break;
        pthread_mutex_unlock((niveles+nivel));
        
        int espera = 1;
        
        while(espera)
        {
            pthread_mutex_lock((niveles+nivel));
            if(*(pesosActuales+nivel)+peso > nivelPeso)
            {
                printf("---- Soy el robot %d y tengo que esperar a que bajen\n",id);
                pthread_cond_wait((sube_t+nivel), (niveles+nivel));
            }
            else
            {
                *(pesosActuales+nivel) += peso;
                printf("^^^^Soy el robot %d y Entre al nivel %d y su peso es de: %d\n",id,nivel,*(pesosActuales+nivel));
                printf("++++ Soy el robot %d haciendo mis compras\n",id);
                espera = 0;
            }
            pthread_mutex_unlock((niveles+nivel));
        }
        
        usleep(rand() % 900);
        
        pthread_mutex_lock((niveles+nivel));
        printf("**** Soy el robot %d, termine en el nivel %d\n", id,nivel);
        *(pesosActuales+nivel) = *(pesosActuales+nivel)-peso;
        pthread_cond_broadcast((sube_t+nivel));
        pthread_mutex_unlock((niveles+nivel));
        ++nivel;
        
        
    }
    
    pthread_exit(NULL);
}


