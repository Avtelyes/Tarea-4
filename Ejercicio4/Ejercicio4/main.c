//
//  main.c
//  Ejercicio4
//
//  Created by Josué Carlos García Puig on 29/10/15.
//  Copyright © 2015 Josue Garcia. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define UNIDADES 4
#define ALARMA 45

void * datos(void *);
void * unidad_central(void *);
void raise_alarm(int);
void escritura(int );
void apertura_archivo();
void cerrar_archivo(int);
int * muestreo;
int * adquisicion;

int termino = 1, conteo = 0;

pthread_mutex_t * mutex_sensor;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t muestra_cond = PTHREAD_COND_INITIALIZER;

pthread_t * sensores;
pthread_t UCR;

FILE * lecturas;

int main(int argc, const char * argv[]) {
    
    srand((int)time(NULL));
    
    sensores = (pthread_t *) malloc(sizeof(pthread_t) * UNIDADES);
    mutex_sensor = (pthread_mutex_t * ) malloc(sizeof(pthread_mutex_t) * UNIDADES);
    muestreo = (int *) malloc(sizeof(int) * UNIDADES);
    adquisicion = (int *) malloc(sizeof(int) * UNIDADES);
    
    apertura_archivo();
    
    signal(SIGINT,cerrar_archivo);
    
    int i = 0;
    
    for (; i < UNIDADES; ++i)
    {
        pthread_mutex_init(mutex_sensor+i,NULL);
    }
    
    pthread_create(&UCR,NULL,unidad_central,NULL);
    
    for (i = 0; i < UNIDADES; ++i)
    {
        *(adquisicion+i) = 0;
        pthread_create(sensores+i,NULL,datos, (void *) i);
    }
    
    
    for (i = 0; i < UNIDADES; ++i)
    {
        pthread_join(*(sensores+i),NULL);
    }
    
    pthread_join(UCR,NULL);
    
    return 0;
}

void * unidad_central(void * arg){
    
    printf("---- CREANDO EL UCR \n");
    signal(SIGUSR1,raise_alarm);
    
    while (termino)
    {
        escritura(conteo);
        printf("%Lectura realizada \n");
        sleep(3);
    }
    
    pthread_exit(NULL);
}

void *datos(void *arg){
    int id = (int)arg;
    printf("------Iniciando el sensor %d-------\n",id);
    int random;
    while (1)
    {
        random = rand() % 50;
        pthread_mutex_lock(mutex_sensor+id);
        *(muestreo+id) = random;
        pthread_mutex_unlock(mutex_sensor+id);
        if (random > ALARMA){
            pthread_mutex_lock(mutex_sensor+id);
            *(adquisicion+id) = 1;
            pthread_kill(UCR,SIGUSR1);
            pthread_mutex_unlock(mutex_sensor+id);
        }
        int s = sleep(1);
    }
    pthread_exit(NULL);
}

void raise_alarm(int id){
    
    int i = 0;
    while (i < UNIDADES)
    {
        pthread_mutex_lock(&mutex_sensor[i]);
        if (*(adquisicion+i)){
            pthread_mutex_lock(&file_mutex);
            fprintf(lecturas,"Central: Recibi valor critico %d de %d\n",*(muestreo+i),i);
            pthread_mutex_unlock(&file_mutex);
            *(adquisicion+i) = 0;
        }
        pthread_mutex_unlock(&mutex_sensor[i]);
        ++i;
    }
}

void escritura(int conteo)
{
    int i = 0;
    
    for (; i < UNIDADES; ++i)
    {
        pthread_mutex_lock(mutex_sensor+i);
        
        fprintf(lecturas,"Lectura del sensor %d: %d\n",i,*(muestreo+i));
        
        pthread_mutex_unlock(mutex_sensor+i);
    }
}

void apertura_archivo()
{
    lecturas = fopen("lecturas.txt", "w+");
    
    if (lecturas == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
}

void cerrar_archivo(int id)
{
    fclose(lecturas);
    free(mutex_sensor);
    free(muestreo);
    free(adquisicion);
    raise(SIGTERM);
}
