#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>
#include <pthread.h>

char password[256];
char a[256];
char s[256];
char h[256];
int f = 0;
int n = 0;

long long total = 0;
pthread_mutex_t mut;
int count_tread = 1;

long long block_len;  //пакеты одинаковые и в пакетах лежат эти idx, каждый такой idx отвечает за формирование своего кодового слова и поток работая на одном пакете переберёт их сразу 1024.

long long cur_block = 0;

void* thread_fun(void* arg) {
    char cur[256];
    struct crypt_data cdata;
    cdata.initialized = 0;

    int a_len = strlen(a);

    while (1) {
        pthread_mutex_lock(&mut);
        if ((cur_block * block_len) >= total || f) {
            pthread_mutex_unlock(&mut);
            break;
        }
        
        
        long long starts = cur_block * block_len;
        cur_block++;

        long long ends = starts + block_len;

        if (ends > total) {
            ends = total;
        }

        pthread_mutex_unlock(&mut);

        //внутри блока перебираем idx
        for (long long i = starts; i < ends; i++) {
            if (f) break;

            long long idx = i;
            for (int pos = n - 1; pos >= 0; pos--) {
                cur[pos] = a[idx % a_len];
                idx /= a_len;
            }
            cur[n] = '\0';


            char* res = crypt_r(cur, s, &cdata);
            if (res && strcmp(res, h) == 0) {
                pthread_mutex_lock(&mut);
                if (!f) {
                    f = 1;
                    strcpy(password, cur);
                }
                pthread_mutex_unlock(&mut);
            
                break;
            }
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int opt;

    a[0] = '\0';
    s[0] = '\0';
    h[0] = '\0';
    password[0] = '\0';
    count_tread = 1;

    while ((opt = getopt(argc, argv, "a:n:h:s:j:")) != -1) {
        if (opt == 'a') strcpy(a, optarg);
        else if (opt == 'n') n = atoi(optarg);
        else if (opt == 'h') strcpy(h, optarg);
        else if (opt == 's') strcpy(s, optarg);
        else if (opt == 'j') {
            count_tread = atoi(optarg);
            if (count_tread < 1) return 1;
        }
    }

    int a_len = strlen(a);
    total = 1;
    for (int i = 0; i < n; i++) total *= a_len;
    
    block_len = total / (count_tread * 8); 
    if (block_len < 256) block_len = 256;
    if (block_len > 65536) block_len = 65536;

    pthread_mutex_init(&mut, NULL);
    cur_block = 0;
    f = 0;

    pthread_t* threads = malloc(count_tread * sizeof(pthread_t));
    if (!threads) {
        pthread_mutex_destroy(&mut);
        return 1;
    }

    for (int t = 0; t < count_tread; t++) {
        pthread_create(&threads[t], NULL, thread_fun, NULL);
    }

    for (int t = 0; t < count_tread; t++) {
        pthread_join(threads[t], NULL);
    }

    free(threads);
    pthread_mutex_destroy(&mut);

    if (f) {
        printf("%s\n", password);
        return 0;
    }
    else {
        fprintf(stderr, "Пароль не найден\n");
        return 1;
    }
}
