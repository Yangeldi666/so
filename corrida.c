#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdatomic.h>
#include<unistd.h>
#include<time.h>

#define META 100
#define BARRA 30

int num_cavalos;
pthread_barrier_t largada;
atomic_int vencedor = -1;
int progresso[16];
pthread_mutex_t tela;


void desenhar_tela(){
    printf("\033[H\033[J");   

    printf(" --- Corrida em andamento ---\n");
    for(int i = 0; i < num_cavalos; i++){
        int p = progresso[i];
        int cheios = (p * BARRA) / META;
        int vazios = BARRA - cheios;

        printf("🐴 Cavalo %2d [", i + 1);
        for(int j = 0; j < cheios; j++) printf("█");
        for(int j = 0; j < vazios; j++) printf("░");
        printf("] %3d%%\n", p);
    }
    fflush(stdout);
}


void *cavalo(void *arg) {
    int id = *(int *)arg;
    srand(time(NULL) ^ id);

    pthread_barrier_wait(&largada);

    for(int i = 0; i<META; i++){
        progresso[id] = i+1;
        pthread_mutex_lock(&tela);
        desenhar_tela();
        pthread_mutex_unlock(&tela);
        usleep(rand() % 80000);
    }
    int esperado = -1;
    if (atomic_compare_exchange_strong(&vencedor, &esperado, id)){
        pthread_mutex_lock(&tela);
        desenhar_tela();
        printf("Cavalo %d venceu a corrida!\n", id+1);
        pthread_mutex_unlock(&tela);
    }

    return NULL;
}


int main(void) {
    printf("Digite a quantidade de cavalos competidores:\n");
    scanf("%d", &num_cavalos);
    while(num_cavalos < 2 || num_cavalos > 16){
        printf("Quantidade de competidores invalida\n");
        scanf("%d", &num_cavalos);
    }

    pthread_t *cavalos = malloc(sizeof(pthread_t) * num_cavalos);
    int *ids = malloc(sizeof(int) * num_cavalos);
    if (cavalos == NULL || ids == NULL){
        perror("Erro ao alocar memoria");
        exit(1);
    }

    pthread_mutex_init(&tela, NULL);
    pthread_barrier_init(&largada, NULL, num_cavalos);
   
    printf("\n");
    printf(" --- Corrida em andamento ---\n");
    for (int i = 0; i < num_cavalos; i++) {
        printf("🐴 Cavalo %2d [%-*s]   0%%\n", i+1, BARRA, "");
    }

    for(int i = 0 ; i<num_cavalos; i++){
        ids[i]=i;
        pthread_create(&cavalos[i], NULL, cavalo, &ids[i]);
    }
    for(int i = 0; i<num_cavalos; i++){
        pthread_join(cavalos[i], NULL);
    }

    printf("\nResultado final: Cavalo %d venceu a corrida!\n", atomic_load(&vencedor)+1);

    pthread_barrier_destroy(&largada);
    pthread_mutex_destroy(&tela);
    free(cavalos);
    free(ids);


    return 0;

}