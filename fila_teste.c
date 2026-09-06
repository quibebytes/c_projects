#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FILA_IMPLEMENTACAO
#include "fila.h"

#define TAMANHO_ENTRADA 16

int main() {
    Fila fila = { .inicio = NULL, .fim = NULL };
    char entrada[TAMANHO_ENTRADA] = {0};
    int opcao = 0;
    
    do {
        printf(
            "1) Obter inicio da fila\n"
            "2) Adicionar elemento na fila\n"
            "3) desenfileirar\n"
            "4) Imprimir fila inteira\n"
            "5) Esvaziar fila por completo\n"
            "6) SAIR\n"
            ":"
        );
        fgets(entrada, TAMANHO_ENTRADA, stdin);
        sscanf(entrada, "%d", &opcao);
        
        switch (opcao) {
            case 1: {
                No *inicio = obter_inicio(&fila);
                if (inicio) {
                    printf("Valor: %d\n", inicio->valor);
                } else {
                    printf("Fila vazia\n");
                }
            } break;
            case 2: {
                int valor = 0;
                printf("Digite o valor a ser enfileirado:");
                fgets(entrada, TAMANHO_ENTRADA, stdin);
                sscanf(entrada, "%d", &valor);
                
                enfileirar(&fila, valor);
            } break;
            case 3: {
                No *no = desenfileirar(&fila);
                
                if (no) {
                    printf("Valor: %d\n", no->valor);
                } else {
                    printf("Fila vazia\n");
                }
                
                free(no);
            } break;
            case 4: {
                imprimir_fila(&fila);
            } break;
            case 5: {
                destruir_fila(&fila);
                printf("Fila esvaziada\n");
            } break;
            case 6: break;
            default: printf("Opcao invalida\n"); break;
        }
        
    } while (opcao != 6);
    
    destruir_fila(&fila);
    
    return 0;
}
