#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>

char password[256];
char a[256];
char s[256];
char h[256];
int f = 0;
int n = 0;

int main(int argc, char* argv[]) {
    int opt;

    a[0] = '\0';
    s[0] = '\0';
    h[0] = '\0';
    password[0] = '\0';

    while ((opt = getopt(argc, argv, "a:n:h:s:")) != -1) {
        if (opt == 'a') {
            strcpy(a, optarg);
        }
        else if (opt == 'n') {
            n = atoi(optarg);
        }
        else if (opt == 'h') {
            strcpy(h, optarg);
        }
        else if (opt == 's') {
            strcpy(s, optarg);
        }
    }


    long long total = 1;
    int a_len = strlen(a);
    for (int i = 0; i < n; i++) {
        total *= a_len;
    }

    char cur[n + 1];
    for (int i = 0; i < n; i++) {
        cur[i] = ' ';
    }
    cur[n] = '\0';

    for (long long i = 0; i < total && !f; i++) {
        long long idx = i;

        for (int pos = n - 1; pos >= 0; pos--) {
            cur[pos] = a[idx % a_len];
            idx /= a_len;
        }

        char* cur_h = (char*)crypt(cur, s);
        if (cur_h && strcmp(cur_h, h) == 0) {
            f = 1;
            strcpy(password, cur);
            break;
        }
    }

    if (f) {
        printf("%s\n", password);
        return 0;
    }
    else {
        fprintf(stderr, "Пароль не найден\n");
        return 1;
    }
}
