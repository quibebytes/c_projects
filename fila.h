#ifndef FILA_H
#define FILA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct No {
    int valor;
    struct No *proximo;
} No;

typedef struct Fila {
    struct No *inicio;
    struct No *fim;
} Fila;

bool fila_vazia(const Fila *fila);
No  *obter_inicio(const Fila *fila);
No  *criar_no_fila(int valor);
No  *desenfileirar(Fila *fila); // chamador eh responsavel pelo free!
void enfileirar(Fila *fila, int valor);
void imprimir_fila(Fila *fila);
void destruir_fila(Fila *fila);

#ifdef FILA_IMPLEMENTACAO

bool fila_vazia(const Fila *fila) {
    return (!fila->inicio || !fila->fim);
}

No *obter_inicio(const Fila *fila) {
    return fila->inicio;
}

No *criar_no_fila(int valor) {
    No *novo = (No *) malloc(sizeof(No));
    if (!novo) {
        fprintf(stderr, "ERRO DE ALOCACAO criar_no_fila()\n");
        exit(EXIT_FAILURE);
    }
    
    novo->proximo = NULL;
    novo->valor = valor;
    
    return novo;
}

void enfileirar(Fila *fila, int valor) {
    if (fila_vazia(fila)) {
        No *novo = criar_no_fila(valor);
        
        fila->inicio = novo;
        fila->fim = novo;
    } else {
        No *novo = criar_no_fila(valor);
        fila->fim->proximo = novo;
        fila->fim = novo;
    }
}

No *desenfileirar(Fila *fila) {
    if (fila->inicio) {
        No *ptr = fila->inicio;
        fila->inicio = fila->inicio->proximo;
        if (!fila->inicio) { fila->fim = NULL; }
        return ptr;
    } else {
        return NULL;
    }
}

void imprimir_fila(Fila *fila) {
    if (fila_vazia(fila)) {
        printf("Fila vazia\n");
    }

    for (No *ptr = fila->inicio; ptr != NULL; ptr = ptr->proximo) {
        printf(
            "Endereco: %p\n"
            "Valor:    %d\n"
            "Proximo:  %p\n"
            "--------------------------\n",
            ptr,
            ptr->valor,
            ptr->proximo
        );
    }
}

void destruir_fila(Fila *fila) {
    No *ptr = fila->inicio;
    while (ptr != NULL) {
        No *proximo = ptr->proximo;
        free(ptr);
        ptr = proximo;
    }
    fila->inicio = NULL;
    fila->fim = NULL;
}
#endif // FILA_IMPLEMENTACAO

#endif // FILA_H