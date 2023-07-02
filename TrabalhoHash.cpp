#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define SEED    0x12345678

typedef struct {
    char *codigoIBGE;
    char nome[50];
    double latitude;
    double longitude;
    int codUF;
    int capital;
    char codSiafi[10];
    int ddd;
    char fusoHorario[70];
} municipio;

char *get_key(void *reg) {
    return (char*)((*((municipio *)reg)).codigoIBGE);
}

void *aloca_municipio(char *ibge, char *nome, double lat, double longi, int capital, int coduf, char *siafi, int ddd, char *fusoH) {
    municipio *cidade = (municipio *)malloc(sizeof(municipio));
    strcpy(cidade->nome, nome);
    strcpy(cidade->codSiafi, siafi);
    strcpy(cidade->fusoHorario, fusoH);
    cidade->codigoIBGE = ibge;
    cidade->latitude = lat;
    cidade->longitude = longi;
    cidade->codUF = coduf;
    cidade->capital = capital;
    cidade->ddd = ddd;
    printf("%s %s %.2f %.2f %d %d %s %d %s\n", cidade->codigoIBGE, cidade->nome, cidade->latitude, cidade->longitude, cidade->capital, cidade->codUF, cidade->codSiafi, cidade->ddd, cidade->fusoHorario);
    return cidade;
}

typedef struct {
    uintptr_t *table;
    int size;
    int max;
    uintptr_t deleted;
    char *(*get_key)(void *);
} thash;

uint32_t hashf(const char *str, uint32_t h) {
    /* One-byte-at-a-time Murmur hash
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

uint32_t hashfDois(const char *str, uint32_t h) {
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x3d4c52e7;
        h ^= h >> 15;
    }
    return h;
}

int hash_insere(thash *h, void *bucket) {
    uint32_t hash = hashf(h->get_key(bucket), SEED);//recebe o valor do codigoibge
    int pos = hash % (h->max);//realiza o calculo para encontrar a posição na tabela onde supostamente deve ser inserido
    uint32_t hash2 = hashfDois(h->get_key(bucket), SEED);//recebe o valor do codigoibge para o segundo hash
    int aux = 1;
    if (h->max == (h->size + 1)) {// a tabela está cheia, por tanto falha a execução
        free(bucket);
        return EXIT_FAILURE;
    } else {
        while (h->table[pos] != 0) {//posição ocupada
            if (h->table[pos] == h->deleted)//posição marcada(livre pois removeu-se um elemento, não precisa calcular nova posição)
                break;
            pos = (pos + aux * hash2) % h->max;//Caso preciso, calcular nova posição(hash duplo)
            aux++;
        }
        h->table[pos] = (uintptr_t)bucket;
        h->size += 1;
    }
    return EXIT_SUCCESS;
}

int hash_constroi(thash *h, int nbuckets, char *(*get_key)(void *)) {
    h->table = (uintptr_t *)calloc(sizeof(uintptr_t), nbuckets + 1);
    if (h->table == NULL) {
        return EXIT_FAILURE;
    }
    h->max = nbuckets + 1;
    h->size = 0;
    h->deleted = (uintptr_t) &(h->size);
    h->get_key = get_key;
    return EXIT_SUCCESS;
}

void *hash_busca(thash h, const char *key) {
    int pos = hashf(key, SEED) % (h.max);
    uint32_t hash2 = hashfDois(key, SEED);
    int aux = 1;
    while (h.table[pos] != 0) {
        if (strcmp(h.get_key((void *)h.table[pos]), key) == 0)
            return (void *)h.table[pos];
        else
            pos = (pos + aux * hash2) % h.max;
        aux++;
    }
    return NULL;
}

int hash_remove(thash *h, const char *key) {
    int pos = hashf(key, SEED) % (h->max);
    while (h->table[pos] != 0) {
        if (strcmp(h->get_key((void *)h->table[pos]), key) == 0) {
            free((void *)h->table[pos]);
            h->table[pos] = h->deleted;
            h->size -= 1;
            return EXIT_SUCCESS;
        } else {
            pos = (pos + 1) % h->max;
        }
    }
    //return EXIT_FAILURE;
}

void hash_apaga(thash *h) {
    int pos;
    for (pos = 0; pos < h->max; pos++) {
        if (h->table[pos] != 0) {
            if (h->table[pos] != h->deleted) {
                free((void *)h->table[pos]);
            }
        }
    }
    free(h->table);
}


int main(int argc, char *argv[]) {
    thash tabhash;
    municipio *cidade;
    int bucket = 100;

    hash_constroi(&tabhash, bucket, get_key);
    hash_insere(&tabhash, aloca_municipio("5200050", "Abadia de Goiás", -16.7573, -49.4412, 0, 52, "1050", 62, "America/Sao_Paulo"));
    return 0;
}
