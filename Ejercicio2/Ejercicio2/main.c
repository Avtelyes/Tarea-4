//
//  main.c
//  Ejercicio2
//
//  Created by Josué Carlos García Puig on 29/10/15.
//  Copyright © 2015 Josue Garcia. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define HOMBRES 10
#define MUJERES 10

int sanitario = 0, numHombres = 0, numMujeres = 0;
int dentroMujeres = 0, dentroHombres = 0;

pthread_mutex_t mutex_sanitatio = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numMu_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numHo_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t denMu_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t denHo_m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sanitario_m = PTHREAD_COND_INITIALIZER;
pthread_cond_t sanitario_h = PTHREAD_COND_INITIALIZER;

void * mujer_quiere_entrar(void *);
void * hombre_quiere_entrar(void *);
void mujer_sale();
void hombre_sale();
void estadoSanitario();

int main(int argc, const char * argv[]) {
    
    srand((int)time(NULL));
    
    int npersonas = HOMBRES + MUJERES;
    
    pthread_t * personas = (pthread_t *) malloc(sizeof(pthread_t) * npersonas);
    
    pthread_t * aux;
    
    int indice = 0;
    
    pthread_mutex_lock(&mutex_sanitatio);
    
    estadoSanitario();
    
    pthread_mutex_unlock(&mutex_sanitatio);
    
    for(aux=personas; aux < (personas+HOMBRES); ++aux )
    {
        ++indice;
        pthread_create(aux, NULL, hombre_quiere_entrar, (void *) indice);
    }
    
    indice = 0;
    
    for(; aux < (personas+npersonas); ++aux )
    {
        ++indice;
        pthread_create(aux, NULL, mujer_quiere_entrar, (void *) indice);
    }
    
    /* Adjuntar los hilos */
    for (aux = personas; aux < (personas+npersonas); ++aux) {
        pthread_join(*aux, NULL);
    }
    
    pthread_mutex_lock(&mutex_sanitatio);
    
    estadoSanitario();
    
    pthread_mutex_unlock(&mutex_sanitatio);
    
    free(personas);
    
    return 0;
}

void * mujer_quiere_entrar(void * arg)
{
    int id = (int) arg;
    //sleep(rand() % 3 + 2);
    //sleep(1);
    usleep(rand() % 500);
    
    pthread_mutex_lock(&numMu_m);
    
    ++numMujeres;
    printf("Llega una mujer (%d en espera)\n", numMujeres);
    
    pthread_mutex_unlock(&numMu_m);
    
    int espera = 1;
    
    while (espera)
    {
        pthread_mutex_lock(&mutex_sanitatio);
        
        if(sanitario == 0)
        {
            printf("Entra una mujer (%d en espera)\n", --numMujeres);
            espera = 0;
            sanitario = 1;
            //int num = rand() % 1;
            //if(num == 0)
                printf("Sanitario ocupado por mujeres\n");
            
            
            ++dentroMujeres;
            
        }
        else if (sanitario == 1)
        {
            printf("Entra una mujer (%d en espera)\n", --numMujeres);
            espera = 0;
            //int num = rand() % 1;
            //if(num == 0)
                printf("Sanitario ocupado por mujeres\n");
            
            
            ++dentroMujeres;
            
        }
        else if (sanitario == 2)
        {
            int num = rand() % 1;
            if(num == 0)
                printf("Sanitario ocupado por hombres\n");
            pthread_cond_wait(&sanitario_m, &mutex_sanitatio);
        }
        
        pthread_mutex_unlock(&mutex_sanitatio);
    }
    
    usleep(rand() % 50);
    
    mujer_sale();
    
    pthread_exit(NULL);
}

void * hombre_quiere_entrar(void * arg)
{
    int id = (int) arg;
    //sleep(1);
    usleep(rand() % 700);
    //sleep(rand() % 3 + 2);
    
    pthread_mutex_lock(&numHo_m);
    
    ++numHombres;
    printf("Llega un hombre (%d en espera)\n", numHombres);
    
    pthread_mutex_unlock(&numHo_m);
    
    int espera = 1;
    
    while (espera)
    {
        pthread_mutex_lock(&mutex_sanitatio);
        
        if(sanitario == 0)
        {
            
            --numHombres;
            printf("Entra un hombre (%d en espera)\n", numHombres);
            espera = 0;
            sanitario = 2;
            //int num = rand() % 1;
            //if(num == 0)
                printf("Sanitario ocupado por hombres\n");
            
            
            ++dentroHombres;
            
        }
        else if (sanitario == 1)
        {
            int num = rand() % 1;
            if(num == 0)
                printf("Sanitario ocupado por mujeres\n");
            pthread_cond_wait(&sanitario_h, &mutex_sanitatio);
        }
        else if (sanitario == 2)
        {
            --numHombres;
            printf("Entra un hombre (%d en espera)\n", numHombres);
            espera = 0;
            //int num = rand() % 1;
            //if(num == 0)
                printf("Sanitario ocupado por hombres\n");
            
            
            ++dentroHombres;
        }
        
        pthread_mutex_unlock(&mutex_sanitatio);
    }
    
    //sleep(rand() % 3 );
    usleep(0);
    
    hombre_sale();
    
    pthread_exit(NULL);
}

void mujer_sale()
{
    
    pthread_mutex_lock(&mutex_sanitatio);
    --dentroMujeres;
    printf("-> Sale una mujer\n");
    
    if(dentroMujeres == 0)
    {
        //pthread_mutex_lock(&mutex_sanitatio);
        sanitario = 0;
        //printf("Sanitario= %d\n", sanitario);
        //pthread_mutex_unlock(&mutex_sanitatio);
        pthread_cond_broadcast(&sanitario_h);
        printf("Sanitario Vacio\n");
    }
    pthread_mutex_unlock(&mutex_sanitatio);
    
}

void hombre_sale()
{
    
    pthread_mutex_lock(&mutex_sanitatio);
    --dentroHombres;
    printf("-> Sale un hombre\n");
    
    if(dentroHombres == 0)
    {
        //pthread_mutex_lock(&mutex_sanitatio);
        sanitario = 0;
        //pthread_mutex_unlock(&mutex_sanitatio);
        pthread_cond_broadcast(&sanitario_m);
        printf("Sanitario Vacio\n");
    }
    pthread_mutex_unlock(&mutex_sanitatio);
    
}

void estadoSanitario()
{
    //sleep(rand() % 2 + 1);
    if(sanitario == 0)
    {
        printf("-> Sanitario Vacío\n");
        
    }
    else if (sanitario == 1)
    {
        printf("-> Sanitario ocupado por mujeres\n");
    }
    else if (sanitario == 2)
    {
        printf("-> Sanitario ocupado por hombres\n");
    }
}


