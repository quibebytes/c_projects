#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>


int main() {

    srand(time(NULL));

    int cartela[5][5];
    char tabela_bingo[15][5];
    const char letras_bingo[5] = {'B', 'I', 'N', 'G', 'O'};

    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 5; j++) {
            tabela_bingo[i][j] = ' ';
        }
    }

    for (int i = 0; i < 25; i++) {

        bool ja_existe;

        do {
            int i_1 = i / 5, i_2 = i % 5;
            cartela[i_1][i_2] = rand() % 75 + 1;

            ja_existe = false;
            for (int j = 0; j < i && !ja_existe; j++) {
                int j_1 = j / 5, j_2 = j % 5;

                if (cartela[i_1][i_2] == cartela[j_1][j_2]) {
                    ja_existe = true;
                }
            }
        } while(ja_existe);
    }


    bool continuar = true, deu_bingo;
    int jogadas = 0;

    while (continuar) {

        bool ja_foi_tirado;
        int n, n_1, n_2;

        do {
            n = rand() % 75 + 1;
            n_1 = (n-1) / 5;
            n_2 = (n-1) % 5;
            ja_foi_tirado = tabela_bingo[n_1][n_2] == 'O';
        } while(ja_foi_tirado);

        tabela_bingo[n_1][n_2] = 'O';
        jogadas++;


        putchar(' ');
        for (int i = 0; i < 5; i++) {
            printf("%5c", letras_bingo[i]);
        }
        putchar('\n');

        for (int i = 0; i < 15; i++) {
            putchar('|');
            for (int j = 0; j < 5; j++) {
                printf(" %c%3d", tabela_bingo[i][j], i*5+j+1);
            }
            printf("|\n");
        }

        printf("O numero %2d foi tirado.\n", n);
        printf("Pressione Enter para continuar:\n");
        getchar();

        printf("CARTELA\n");
        for (int i = 0; i < 5; i++) {
            putchar('|');
            for (int j = 0; j < 5; j++) {
                int cartela_1 = (cartela[i][j]-1) / 5;
                int cartela_2 = (cartela[i][j]-1) % 5;

                printf(" %c%3d", (tabela_bingo[cartela_1][cartela_2] == 'O')? 'X':' ', cartela[i][j]);
            }
            printf("|\n");
        }


        deu_bingo = true;

        for (int i = 0; i < 5 && deu_bingo; i++) {
            for (int j = 0; j < 5 && deu_bingo; j++) {
                int cartela_1 = (cartela[i][j]-1) / 5;
                int cartela_2 = (cartela[i][j]-1) % 5;

                deu_bingo = tabela_bingo[cartela_1][cartela_2] == 'O';
            }
        }

        if (deu_bingo) {
            continuar = false;
        } else {
            printf("\nPressione Enter para continuar:\n");
            getchar();
        }
    }

    printf("Bingo!! Voce ganhou em %d jogadas.\n", jogadas);

    return 0;
}