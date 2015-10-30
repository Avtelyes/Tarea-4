//
//  main.c
//  Ejercicio1
//
//  Created by Josué Carlos García Puig on 28/10/15.
//  Copyright © 2015 Josue Garcia. All rights reserved.
//

/*
 ANÁLISIS
 
 El problema de concurrencia que presenta el caso de Blancanieves es el recurso de la mesa, cada enano que llega debe de obtener el lugar para comer, sino tendrán que esperarse. Esto causa un recurso compartido por los 7 enanos al igual que la espera de la comida por parte de blancanieves que debe de servir de acuerdo a como van llegando.
 Por estos recursos compartidos debe de existir una sincronización, definir el momento en el que llega un enano y ocupa una mesa seguido por el servicio de comida y se retira para que otro ocupe su lugar.
 
 La técnica elegida para este problema es la de semaforos de exclusión y de señal, esto para separar los recursos de los enanos que estan sentados y los que estan esperando. Tambien enviar para enviar una señal a Blancanieves cuando ya se sienta para que le sirvan y si no hay nadie a quien servir que vaya a estudiar Programacion Avanzada.
*/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define ENANITOS 7
#define HAMBRE 2

int mesa = 0, comida = 0, servidos = 0;

pthread_mutex_t mutex_mesa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_servidos = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sentarse_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t servir_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t esperar_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t comida_t = PTHREAD_COND_INITIALIZER;

void * dwarfs(void *);
void * snowwhite(void *);
void extraerDiamantes(int);
int intentoSentarse(int);
void servirComida();

int main(int argc, const char * argv[]) {
    
    srand((int) time(NULL));
    
    pthread_t * enanos = (pthread_t *) malloc (sizeof(pthread_t) * ENANITOS);

    pthread_t * blancanieves = (pthread_t *) malloc(sizeof(pthread_t));
    
    pthread_t * aux;
    int indice = 0;
    
    /* Crear los enanitos */
    for (aux = enanos; aux < (enanos+ENANITOS); ++aux)
    {
        printf("--- Creando el enano %d ---\n", ++indice);
        pthread_create(aux, NULL, dwarfs, (void *) indice);
    }
    
    /* Crear a blancanieves */
    
    pthread_create(blancanieves, NULL, snowwhite, NULL);
    
    /* Adjuntar los hilos */
    for (aux = enanos; aux < (enanos+ENANITOS); ++aux) {
        pthread_join(*aux, NULL);
    }
    
    pthread_join(*blancanieves, NULL);
    
    free(enanos);
    free(blancanieves);
    
    return 0;
    
}

void * dwarfs(void * arg)
{
    int id = (int) arg;
    int hambre = HAMBRE;
    
    while (1) {
        
        extraerDiamantes(id);
        
        if(hambre == 0)
        {
            printf("Soy el enano %d y ya no comeré más\n",id);
            break;
        }
        pthread_mutex_lock(&mutex_mesa);
        while(!intentoSentarse(id))
        {
            printf("--- Enano %d, esperando turno en la mesa\n",id);
            pthread_cond_wait(&sentarse_t, &mutex_mesa);
            //printf("UUUU SOy el Enano %d, listo para buscar silla\n",id);
        }
  
        pthread_mutex_unlock(&mutex_mesa);
        
        pthread_cond_signal(&esperar_t);
        
        pthread_mutex_lock(&mutex_servidos);
        ++servidos;
        printf("Enano %d, pidiendo comida\n",id);
        pthread_cond_wait(&comida_t, &mutex_servidos);
        --hambre;
        pthread_mutex_unlock(&mutex_servidos);
        
        printf("**** Enano %d comiendo\n", id);
        sleep(rand() % 3 + 1);
        
        pthread_mutex_lock(&mutex_mesa);
        
        printf("^^^^ Enano %d me paré de la mesa\n",id);
        --mesa;
        
        pthread_mutex_unlock(&mutex_mesa);
        //sleep(2);

        pthread_cond_broadcast(&sentarse_t);
        
    }
    
    pthread_exit(NULL);
}


void * snowwhite(void * arg)
{
    int contados = 0;
    while (1)
    {
        //printf("CONTADOS %d\n",contados);
        if(contados == HAMBRE*7)
            break;
        
        pthread_mutex_lock(&mutex_mesa);
        
        if(mesa == 0)
        {
            printf("--- Blancanieves estudiando Programacion Avanzada\n");
            pthread_cond_wait(&esperar_t, &mutex_mesa);
        }
            
        pthread_mutex_unlock(&mutex_mesa);
        
        pthread_mutex_lock(&mutex_servidos);
        if(servidos > 0)
        {
            //printf("SERVIDOS %d\n",servidos);
            servirComida();
            ++contados;
            --servidos;
            //printf("SERVIDOS %d\n",servidos);
            pthread_cond_signal(&comida_t);
        }
        pthread_mutex_unlock(&mutex_servidos);
    }
    
    pthread_exit(NULL);
}

void extraerDiamantes(int enano)
{
    printf("Soy el enano %d y estoy extrayendo diamantes en la mina\n",enano);
    sleep(3);
}

int intentoSentarse(int enano)
{

    
    if(mesa != 4)
    {
        printf("++++ Enano %d sentado en la mesa\n", enano);
        ++mesa;
        return 1;
    }
    else
        return 0;
}

void servirComida()
{
    printf("Blancanieves sirviendo comida\n");
    sleep(rand() % 2 + 1);
}


