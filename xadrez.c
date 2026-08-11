/*
Features:
    detecta se cada jogada eh legal
    xeque
    xeque-mate
    peoes podem dar dois passos
    promocoes
    en passant
    roques menor e maior

TODO pra dps pq chega, adicionar modo de debug com mais prints (debugar esse ngc ta incoveniente, droga de recursao!)
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

//#define MODO_UTF8 // Comente essa parte para somente utilizar caracteres ASCII

#ifdef MODO_UTF8
#include<locale.h>
#endif

#define DENTRO_DO_TABULEIRO(x, y) ((x) > 0 && (x) <= 8 && (y) > 0 && (y) <= 8)

#define VALIDA_TORRE(x, y, t_x, t_y) ((t_x) == (x) || (t_y) == (y))

#define VALIDA_BISPO(x, y, t_x, t_y) (abs((t_x) - (x)) == abs((t_y) - (y)))

#define VALIDA_DAMA(x, y, t_x, t_y) (VALIDA_BISPO(x, y, t_x, t_y) || VALIDA_TORRE(x, y, t_x, t_y))

#define VALIDA_CAVALO(x, y, t_x, t_y) ( \
    !VALIDA_DAMA(x, y, t_x, t_y) && \
    abs((t_x) - (x)) <= 2 && \
    abs((t_y) - (y)) <= 2 \
)

#define VALIDA_REI(x, y, t_x, t_y) ( \
    VALIDA_DAMA(x, y, t_x, t_y) && \
    abs((t_x) - (x)) < 2 && \
    abs((t_y) - (y)) < 2 \
)


typedef enum Espaco {
    ESPACO_PEAO_1 = 0,
    ESPACO_CAVALO_1,
    ESPACO_BISPO_1,
    ESPACO_TORRE_1,
    ESPACO_DAMA_1,
    ESPACO_REI_1,

    ESPACO_VAZIO = 6,

    ESPACO_PEAO_2 = 7,
    ESPACO_CAVALO_2,
    ESPACO_BISPO_2,
    ESPACO_TORRE_2,
    ESPACO_DAMA_2,
    ESPACO_REI_2
} Espaco;

// flags para determinar certas informacoes que nao podem ser calculadas so de olhar o tabuleiro
// honestamente deveria ter sido uma fat struct
typedef enum EstadoJogo {
    ESTADO_JOGADOR = 1, // 0 = primeiro, 1 = segundo
    ESTADO_ROQUE_MENOR_1 = 1 << 1, // quais roques sao permitidos
    ESTADO_ROQUE_MAIOR_1 = 1 << 2,
    ESTADO_ROQUE_MENOR_2 = 1 << 3,
    ESTADO_ROQUE_MAIOR_2 = 1 << 4,
    ESTADO_ROQUES = ESTADO_ROQUE_MENOR_1 | ESTADO_ROQUE_MAIOR_1 | ESTADO_ROQUE_MENOR_2 | ESTADO_ROQUE_MAIOR_2,
    ESTADO_SALTO = 1 << 5, // quando o peao da dois passos
    ESTADO_SALTO_POSICAO = (1 << 6) + (1 << 7) + (1 << 8), // de que coluna saiu o peao dos dois passos (0 a 7, incluindo ambos)
} EstadoJogo;

#ifdef MODO_UTF8
char *espaco_para_char(Espaco espaco);
#else
char espaco_para_char(Espaco espaco);
#endif

int aplicar_mascara(EstadoJogo estado_jogo, EstadoJogo mascara);
bool jogada_eh_valida(Espaco tabuleiro[8][8], EstadoJogo estado_jogo, int x, int y, int t_x, int t_y);
void atualizar_estado_jogo(Espaco tabuleiro[8][8], EstadoJogo *estado_jogo, int x, int y, int t_x, int t_y);
bool em_xeque(Espaco tabuleiro[8][8], EstadoJogo estado_jogo);
bool esta_bloqueada(Espaco tabuleiro[8][8], int x, int y, int t_x, int t_y);
void imprimir_tabuleiro(Espaco tabuleiro[8][8], int jogador);
void conseguir_coordenadas(char *entrada, int bufsiz, int *x, int *y);


int main() {
    #ifdef MODO_UTF8
    char *locale = setlocale(LC_CTYPE, "C.UTF-8");
    if (!locale) {
        puts("Usando local preferido da maquina.\n");
        setlocale(LC_CTYPE, "");
    }
    #endif

    char entrada[BUFSIZ];
    char msg_vazia[] = "";
    char msg_jogada_invalida[] = "Jogada invalida, tente outra.";
    char msg_xeque[] = "Xeque!";
    char msg_promocao[] = "Um peao foi promovido.";
    char msg_en_passant[] = "En passant!";
    char msg_roque_menor[] = "Roque menor.";
    char msg_roque_maior[] = "Roque maior.";
    char *msg_atual = msg_vazia;
    Espaco tabuleiro[8][8];
    Espaco validas[8][8];
    EstadoJogo estado_jogo = (EstadoJogo) 0;

    estado_jogo |= ESTADO_ROQUE_MENOR_1;
    estado_jogo |= ESTADO_ROQUE_MAIOR_1;
    estado_jogo |= ESTADO_ROQUE_MENOR_2;
    estado_jogo |= ESTADO_ROQUE_MAIOR_2;

    for (int i = 0; i < 8; i++) {
        for (int  j = 0; j < 8; j++) {
            if (i == 1) {
                tabuleiro[i][j] = ESPACO_PEAO_1;
            } else if (i == 6) {
                tabuleiro[i][j] = ESPACO_PEAO_2;
            } else {
                tabuleiro[i][j] = ESPACO_VAZIO;
            }
        }
    }

    tabuleiro[0][0] = ESPACO_TORRE_1; tabuleiro[0][7] = ESPACO_TORRE_1;
    tabuleiro[7][0] = ESPACO_TORRE_2; tabuleiro[7][7] = ESPACO_TORRE_2;
    tabuleiro[0][1] = ESPACO_CAVALO_1; tabuleiro[0][6] = ESPACO_CAVALO_1;
    tabuleiro[7][1] = ESPACO_CAVALO_2; tabuleiro[7][6] = ESPACO_CAVALO_2;
    tabuleiro[0][2] = ESPACO_BISPO_1; tabuleiro[0][5] = ESPACO_BISPO_1;
    tabuleiro[7][2] = ESPACO_BISPO_2; tabuleiro[7][5] = ESPACO_BISPO_2;
    tabuleiro[0][3] = ESPACO_DAMA_1; tabuleiro[0][4] = ESPACO_REI_1;
    tabuleiro[7][3] = ESPACO_DAMA_2; tabuleiro[7][4] = ESPACO_REI_2;


    bool continuar = true;

    puts("Xadrez");
    puts("Digite \"ajuda\" para obter dicas.\n");

    do {
        imprimir_tabuleiro(tabuleiro, aplicar_mascara(estado_jogo, ESTADO_JOGADOR));
        puts(msg_atual);
        printf("Vez do %s.\n", (aplicar_mascara(estado_jogo, ESTADO_JOGADOR))? "preto": "branco");

        int x=0, y=0, t_x=0, t_y=0;
        printf("Digite as coordenadas da peca:\n");
        conseguir_coordenadas(entrada, BUFSIZ, &x, &y);

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                validas[i][j] = (jogada_eh_valida(tabuleiro, estado_jogo, x, y, j+1, i+1))?
                    tabuleiro[y-1][x-1]:
                    ESPACO_VAZIO
                ;
            }
        }

        imprimir_tabuleiro(validas, aplicar_mascara(estado_jogo, ESTADO_JOGADOR));

        printf("Digite as coordenadas da jogada:\n");
        conseguir_coordenadas(entrada, BUFSIZ, &t_x, &t_y);

        bool jogada_valida = jogada_eh_valida(tabuleiro, estado_jogo, x, y, t_x, t_y);

        if (jogada_valida) {
            msg_atual = msg_vazia;
            tabuleiro[t_y-1][t_x-1] = tabuleiro[y-1][x-1];
            tabuleiro[y-1][x-1] = ESPACO_VAZIO;

            if ((tabuleiro[t_y-1][t_x-1] % 7) == ESPACO_PEAO_1) {
                if (t_y == ((aplicar_mascara(estado_jogo, ESTADO_JOGADOR))? 1: 8)) { // promocao
                    puts("O peao sera promovido.\nDigite o numero da peca desejada.");
                    puts("1 = cavalo\n2 = bispo\n3 = torre\n4 = dama");
                    int peca_nova = 0;
                    bool sucesso = false;
                    do {
                        printf("# ");
                        fgets(entrada, BUFSIZ, stdin);
                        sucesso = sscanf(entrada, "%d", &peca_nova) == 1;

                        if (peca_nova < 1 || peca_nova > 4) {
                            puts("Peca invalida. Tente novamente.");
                            sucesso = false;
                        }

                    } while (!sucesso);

                    tabuleiro[t_y-1][t_x-1] = ESPACO_PEAO_1 + peca_nova + ((aplicar_mascara(estado_jogo, ESTADO_JOGADOR))? 7: 0);
                    msg_atual = msg_promocao;
                }


                bool jogador_da_peca = tabuleiro[t_y-1][t_x-1] > 6;
                if (
                    t_y == y+(jogador_da_peca? -1: 1) &&
                    (t_x == x-1 || t_x == x+1) &&
                    (y == (jogador_da_peca? 4: 5)) &&
                    aplicar_mascara(estado_jogo, ESTADO_SALTO) &&
                    t_x-1 == aplicar_mascara(estado_jogo, ESTADO_SALTO_POSICAO)
                ) {
                    tabuleiro[t_y-1+(jogador_da_peca? 1: -1)][t_x-1] = ESPACO_VAZIO;
                    msg_atual = msg_en_passant;
                }
            }


            bool jogador_da_peca = tabuleiro[t_y-1][t_x-1] > 6;
            EstadoJogo estado_roque_menor = jogador_da_peca? ESTADO_ROQUE_MENOR_2: ESTADO_ROQUE_MENOR_1;
            EstadoJogo estado_roque_maior = jogador_da_peca? ESTADO_ROQUE_MAIOR_2: ESTADO_ROQUE_MAIOR_1;

            if (
                t_x == x+2 && t_y == y &&
                (tabuleiro[t_y-1][t_x-1] % 7 == ESPACO_REI_1) &&
                aplicar_mascara(estado_jogo, estado_roque_menor) &&
                !esta_bloqueada(tabuleiro, x, y, t_x, t_y)
            ) {
                tabuleiro[t_y-1][t_x-2] = jogador_da_peca? ESPACO_TORRE_2: ESPACO_TORRE_1;
                tabuleiro[t_y-1][7] = ESPACO_VAZIO;
                msg_atual = msg_roque_menor;
            }

            if (
                t_x == x-2 && t_y == y &&
                (tabuleiro[t_y-1][t_x-1] % 7 == ESPACO_REI_1) &&
                aplicar_mascara(estado_jogo, estado_roque_maior) &&
                !esta_bloqueada(tabuleiro, t_x, t_y, t_x-2, t_y) &&
                !esta_bloqueada(tabuleiro, x, y, t_x, t_y)
            ) {
                tabuleiro[t_y-1][t_x] = jogador_da_peca? ESPACO_TORRE_2: ESPACO_TORRE_1;
                tabuleiro[t_y-1][0] = ESPACO_VAZIO;
                msg_atual = msg_roque_maior;
            }

            atualizar_estado_jogo(tabuleiro, &estado_jogo, x, y, t_x, t_y);
        } else {
            msg_atual = msg_jogada_invalida;
            estado_jogo ^= ESTADO_JOGADOR; // forcar negacao dupla no final para fazer o jogador jogar de novo
        }

        if (em_xeque(tabuleiro, estado_jogo ^ ESTADO_JOGADOR)) {
            bool inimigo_tem_jogada = false;
            for (int i = 0; i < 8 && !inimigo_tem_jogada; i++) {
                for (int j = 0; j < 8 && !inimigo_tem_jogada; j++) {

                    bool jogador_do_espaco = tabuleiro[i][j] > 6;
                    if (tabuleiro[i][j] != ESPACO_VAZIO && jogador_do_espaco != aplicar_mascara(estado_jogo, ESTADO_JOGADOR)) {

                        for (int k = 0; k < 8 && !inimigo_tem_jogada; k++) {
                            for (int l = 0; l < 8 && !inimigo_tem_jogada; l++) {
                                inimigo_tem_jogada = jogada_eh_valida(tabuleiro, estado_jogo ^ ESTADO_JOGADOR, j+1, i+1, l+1, k+1);
                            }
                        }
                    }
                }
            }

            continuar = inimigo_tem_jogada;
            msg_atual = msg_xeque;
        }

        // pra debug
//        printf("m1: %d, M1: %d, m2: %d, M2: %d\n",
//            aplicar_mascara(estado_jogo, ESTADO_ROQUE_MENOR_1),
//            aplicar_mascara(estado_jogo, ESTADO_ROQUE_MAIOR_1),
//            aplicar_mascara(estado_jogo, ESTADO_ROQUE_MENOR_2),
//            aplicar_mascara(estado_jogo, ESTADO_ROQUE_MAIOR_2)
//        );

        estado_jogo ^= ESTADO_JOGADOR;
    } while(continuar);

    imprimir_tabuleiro(tabuleiro, estado_jogo ^ ESTADO_JOGADOR);
    puts("Xeque-mate!");
    printf("Vitoria do jogador %s.\n", (aplicar_mascara(estado_jogo, ESTADO_JOGADOR))? "branco": "preto");

    return 0;
}


int aplicar_mascara(EstadoJogo estado_jogo, EstadoJogo mascara) {
    while (!(mascara & 1)) {
        estado_jogo >>= 1;
        mascara >>= 1;
    }

    return estado_jogo & mascara;
}

#ifdef MODO_UTF8
char *espaco_para_char(Espaco espaco) {
    static char *chars[] = {
        "♟", "♞", "♝", "♜", "♛", "♚", " ", "♙", "♘", "♗", "♖", "♕", "♔"
    };

    return chars[espaco];
}
#else
char espaco_para_char(Espaco espaco) {
    static const char chars[] = {
        'p', 'C', 'B', 'T', 'D', 'R', ' '
    };

    return chars[espaco % 7];
}
#endif

void conseguir_coordenadas(char *entrada, int bufsiz, int *x, int *y) {
    int sucesso = 0;

    do {
        printf("# ");
        char *resultado = fgets(entrada, bufsiz, stdin);

        for (int i = 0; i < bufsiz && resultado; i++) {
            if (resultado[i] == '\n') {
                resultado[i] = '\0';
            }
        }


        if (!strcmp(entrada, "ajuda")) {
            puts("\nDigite as coordenadas na forma \"<letra> <digito>\", exemplo: d4.\n");
            puts("Se voce digitar uma jogada invalida, o jogo vai reiniciar seu turno,");
            puts("entao se atente de quem eh a vez na mensagem abaixo do tabuleiro!\n");
            puts("Os 1 e 2 denotam o jogador cuja cada peca pertence:");
            puts("1 = branco (primeiro), 2 = preto (segundo)\n");
            puts("Os nomes das pecas estao abreviadas do seguinte modo:");
            puts("p = peao\nC = cavalo\nB = bispo\nT = torre\nD = dama (rainha)\nR = rei");
        } else {
            char c_x;
            sucesso = sscanf(entrada, "%c %d", &c_x, y) == 2;
            *x = ((c_x > 0x60 && c_x <= 0x7A)? c_x - 0x20: c_x) - 0x40;

            if (!sucesso || !DENTRO_DO_TABULEIRO(*x, *y)) {
                puts("Erro ao ler coordenadas, tente de novo.");
                puts("Se estiver com duvidas, digite \"ajuda\".");
                sucesso = 0;
            }
        }
    } while (!sucesso);
}

bool em_xeque(Espaco tabuleiro[8][8], EstadoJogo estado_jogo) {
    bool jogador = aplicar_mascara(estado_jogo, ESTADO_JOGADOR);
    int rei_x = 0, rei_y = 0;
    bool achou_rei = false;
    for (int i = 0; i < 8 && !achou_rei; i++) {
        for (int j = 0; j < 8 && !achou_rei; j++) {
            if (tabuleiro[i][j] == (jogador? ESPACO_REI_2: ESPACO_REI_1)) {
                rei_x = j+1;
                rei_y = i+1;
                achou_rei = true;
            }
        }
    }

    bool xeque = !achou_rei;
    for (int i = 0; i < 8 && !xeque; i++) {
        for (int j = 0; j < 8 && !xeque; j++) {
            bool jogador_do_espaco = tabuleiro[i][j] > 6;
            if (tabuleiro[i][j] != ESPACO_VAZIO && jogador_do_espaco != jogador &&
                jogada_eh_valida(tabuleiro, estado_jogo ^ ESTADO_JOGADOR, j+1, i+1, rei_x, rei_y)
            ) {
                xeque = true;
            }
        }
    }

    return xeque;
}

bool esta_bloqueada(Espaco tabuleiro[8][8], int x, int y, int t_x, int t_y) {
    // A maior parte da complicacao deste codigo vem do fato de q ele tem q funcionar em 8 direcoes
    // A Divisao eh pra manter o sinal mas limitar pra 1, -1 ou 0
    int dif_x = x - t_x;
    int dif_y = y - t_y;
    int inc_x = dif_x / ((dif_x == 0)? 1: abs(dif_x));
    int inc_y = dif_y / ((dif_y == 0)? 1: abs(dif_y));
    int comeco, inc, final, j;

    if (dif_x == 0) {
        comeco = t_y+inc_y;
        inc = inc_y;
        final = y;
        j = t_x+inc_x;
    } else {
        comeco = t_x+inc_x;
        inc = inc_x;
        final = x;
        j = t_y+inc_y;
    }

    bool bloqueada = false;

    for (int i = comeco; i != final && !bloqueada; i += inc) {
        Espaco local;
        if (dif_x == 0) {
            local = tabuleiro[i-1][j-1];
            j += inc_x;
        } else {
            local = tabuleiro[j-1][i-1];
            j += inc_y;
        }

        if (local != ESPACO_VAZIO) { bloqueada = true; }
    }

    return bloqueada;
}

void atualizar_estado_jogo(Espaco tabuleiro[8][8], EstadoJogo *estado_jogo, int x, int y, int t_x, int t_y) {
    if (
        ((tabuleiro[t_y-1][t_x-1] % 7) == ESPACO_PEAO_1) &&
        y == (aplicar_mascara(*estado_jogo, ESTADO_JOGADOR)? 7: 2) &&
        t_y == (aplicar_mascara(*estado_jogo, ESTADO_JOGADOR)? 5: 4)
    ) {
        *estado_jogo |= ESTADO_SALTO;
        int posicao = t_x-1, mascara = ESTADO_SALTO_POSICAO;
        while (!(mascara & 1)) {
            mascara >>= 1;
            posicao <<= 1;
        }
        *estado_jogo &= ~ESTADO_SALTO_POSICAO;
        *estado_jogo |= posicao;
    } else {
        *estado_jogo &= ~ESTADO_SALTO;
    }


    bool jogador_da_peca = tabuleiro[t_y-1][t_x-1] > 6;
    EstadoJogo estado_roque_menor = jogador_da_peca? ESTADO_ROQUE_MENOR_2: ESTADO_ROQUE_MENOR_1;
    EstadoJogo estado_roque_maior = jogador_da_peca? ESTADO_ROQUE_MAIOR_2: ESTADO_ROQUE_MAIOR_1;

    if ((tabuleiro[t_y-1][t_x-1] % 7) == ESPACO_REI_1) {
        *estado_jogo &= ~(estado_roque_menor | estado_roque_maior);
    }

    if (tabuleiro[jogador_da_peca? 7: 0][7] == ESPACO_VAZIO) { *estado_jogo &= ~estado_roque_menor; }
    if (tabuleiro[jogador_da_peca? 7: 0][0] == ESPACO_VAZIO) { *estado_jogo &= ~estado_roque_maior; }

    if ((tabuleiro[jogador_da_peca? 0: 7][7] > 6) == jogador_da_peca &&
        tabuleiro[jogador_da_peca? 0: 7][7] != ESPACO_VAZIO) {
        *estado_jogo &= ~(jogador_da_peca? ESTADO_ROQUE_MENOR_1: ESTADO_ROQUE_MENOR_2);
    }

    if ((tabuleiro[jogador_da_peca? 0: 7][0] > 6) == jogador_da_peca &&
        tabuleiro[jogador_da_peca? 0: 7][7] != ESPACO_VAZIO) {
        *estado_jogo &= ~(jogador_da_peca? ESTADO_ROQUE_MAIOR_1: ESTADO_ROQUE_MAIOR_2);
    }
}

bool jogada_eh_valida(Espaco tabuleiro[8][8], EstadoJogo estado_jogo, int x, int y, int t_x, int t_y) {

    if (x == t_x && y == t_y) { return false; }

    bool jogada_valida = false;
    bool jogador_da_peca = tabuleiro[y-1][x-1] > 6;

    int sentido = jogador_da_peca? -1: 1;
    bool t_esta_vazio = tabuleiro[t_y-1][t_x-1] == ESPACO_VAZIO;

    bool en_passant_valido =
        (tabuleiro[y-1][x-1] % 7) == ESPACO_PEAO_1 &&
        t_y == y+sentido &&
        (t_x == x-1 || t_x == x+1) &&
        (y == (jogador_da_peca? 4: 5)) &&
        aplicar_mascara(estado_jogo, ESTADO_SALTO) &&
        t_x-1 == aplicar_mascara(estado_jogo, ESTADO_SALTO_POSICAO)
    ;


    bool jogador_do_alvo = tabuleiro[t_y-1][t_x-1] > 6;
    bool pode_comer = jogador_da_peca != jogador_do_alvo || t_esta_vazio;
    EstadoJogo estado_roque_menor = jogador_da_peca? ESTADO_ROQUE_MENOR_2: ESTADO_ROQUE_MENOR_1;
    EstadoJogo estado_roque_maior = jogador_da_peca? ESTADO_ROQUE_MAIOR_2: ESTADO_ROQUE_MAIOR_1;

    bool roque_menor_valido =
        t_x == x+2 && t_y == y &&
        (tabuleiro[y-1][x-1] % 7 == ESPACO_REI_1) &&
        aplicar_mascara(estado_jogo, estado_roque_menor) &&
        !esta_bloqueada(tabuleiro, x, y, t_x, t_y)
    ;

    bool roque_maior_valido =
        t_x == x-2 && t_y == y &&
        (tabuleiro[y-1][x-1] % 7 == ESPACO_REI_1) &&
        aplicar_mascara(estado_jogo, estado_roque_maior) &&
        !esta_bloqueada(tabuleiro, t_x, t_y, t_x-2, t_y) &&
        !esta_bloqueada(tabuleiro, x, y, t_x, t_y)
    ;


    switch (tabuleiro[y-1][x-1] % 7) { // desconsiderar qual jogador
        case ESPACO_PEAO_1:
            jogada_valida =
                (t_y == y+sentido && t_x == x && t_esta_vazio) || // normal
                ( // dois passos
                    (y == (jogador_da_peca? 7: 2)) &&
                    (t_y == y+sentido*2 && t_x == x) &&
                    t_esta_vazio &&
                    !esta_bloqueada(tabuleiro, x, y, t_x, t_y)
                ) || ( // comer
                    t_y == y+sentido &&
                    (t_x == x-1 || t_x == x+1) &&
                    !t_esta_vazio
                ) ||
                en_passant_valido
            ;
            break;
        case ESPACO_TORRE_1:
            jogada_valida = VALIDA_TORRE(x, y, t_x, t_y);
            jogada_valida = jogada_valida && !esta_bloqueada(tabuleiro, x, y, t_x, t_y);
            break;
        case ESPACO_BISPO_1:
            jogada_valida = VALIDA_BISPO(x, y, t_x, t_y);
            jogada_valida = jogada_valida && !esta_bloqueada(tabuleiro, x, y, t_x, t_y);
            break;
        case ESPACO_DAMA_1:
            jogada_valida = VALIDA_DAMA(x, y, t_x, t_y);
            jogada_valida = jogada_valida && !esta_bloqueada(tabuleiro, x, y, t_x, t_y);
            break;
        case ESPACO_CAVALO_1:
            jogada_valida = VALIDA_CAVALO(x, y, t_x, t_y);
            break;
        case ESPACO_REI_1:
            jogada_valida = VALIDA_REI(x, y, t_x, t_y) || roque_menor_valido || roque_maior_valido;
            break;
        default: jogada_valida = false;
    }


    // Codigo meio suspeito mas conveniente
    if (jogada_valida) {

        // se nao fizer isso causa um overflow do stack lolololol
        bool xeque = (roque_menor_valido || roque_maior_valido)? em_xeque(tabuleiro, estado_jogo): false;

        Espaco pos_original = tabuleiro[y-1][x-1];
        Espaco tar_original = tabuleiro[t_y-1][t_x-1];
        Espaco epc_original = tabuleiro[t_y-1-sentido][t_x-1];


        // Modificar o tabuleiro, incluindo os efeitos colaterais

        tabuleiro[y-1][x-1] = ESPACO_VAZIO;
        tabuleiro[t_y-1][t_x-1] = pos_original;
        if (en_passant_valido) { tabuleiro[t_y-1-sentido][t_x-1] = ESPACO_VAZIO; }
        if (roque_menor_valido && !xeque) {
            tabuleiro[t_y-1][t_x-2] = tabuleiro[t_y-1][7];
            tabuleiro[t_y-1][7] = ESPACO_VAZIO;
        }
        if (roque_maior_valido && !xeque) {
            tabuleiro[t_y-1][t_x] = tabuleiro[t_y-1][0];
            tabuleiro[t_y-1][0] = ESPACO_VAZIO;
        }

        atualizar_estado_jogo(tabuleiro, &estado_jogo, x, y, t_x, t_y);
        jogada_valida = jogada_valida && !em_xeque(tabuleiro, estado_jogo);


        // Retornar tudo do jeito de antes

        tabuleiro[y-1][x-1] = pos_original;
        tabuleiro[t_y-1][t_x-1] = tar_original;
        if (en_passant_valido) { tabuleiro[t_y-1-sentido][t_x-1] = epc_original; }
        if (roque_menor_valido && !xeque) {
            tabuleiro[t_y-1][7] = tabuleiro[t_y-1][t_x-2];
            tabuleiro[t_y-1][t_x-2] = ESPACO_VAZIO;
        }
        if (roque_maior_valido && !xeque) {
            tabuleiro[t_y-1][0] = tabuleiro[t_y-1][t_x];
            tabuleiro[t_y-1][t_x] = ESPACO_VAZIO;
        }
    }


    jogada_valida = jogada_valida && jogador_da_peca == aplicar_mascara(estado_jogo, ESTADO_JOGADOR) && pode_comer;

    return jogada_valida;
}

#ifdef MODO_UTF8
void imprimir_tabuleiro(Espaco tabuleiro[8][8], int jogador) {
    if (jogador) { // segundo jogador
        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf(" %c", 'h' - i);
        }
        putchar('\n');

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("▁▁");
        }
        putchar('\n');

        for (int i = 0; i < 8; i++) {
            printf("%2d▕", i+1);
            for (int j = 7; j >= 0; j--) {
                char *quad = (j%2) ^ (i%2) ? "▓": "░";

                if (tabuleiro[i][j] == ESPACO_VAZIO) {
                    printf("%s%s", quad, quad);
                } else {
                    char *p = espaco_para_char(tabuleiro[i][j]);
                    printf("%s%s", quad, p);
                }
            }
            printf("▏\n");
        }

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("▔▔");
        }
        putchar('\n');
    } else { // primeiro jogador
        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf(" %c", 'a' + i);
        }
        putchar('\n');

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("▁▁");
        }
        putchar('\n');

        for (int i = 7; i >= 0; i--) {
            printf("%2d▕", i+1);
            for (int j = 0; j < 8; j++) {
                char *quad = (j%2) ^ (i%2) ? "▓": "░";

                if (tabuleiro[i][j] == ESPACO_VAZIO) {
                    printf("%s%s", quad, quad);
                } else {
                    char *p = espaco_para_char(tabuleiro[i][j]);
                    printf("%s%s", quad, p);
                }
            }
            printf("▏\n");
        }

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("▔▔");
        }
        putchar('\n');
    }
}
#else
void imprimir_tabuleiro(Espaco tabuleiro[8][8], int jogador) {
    if (jogador) { // segundo jogador
        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("  %c", 'H' - i);
        }
        putchar('\n');

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("___");
        }
        putchar('\n');

        for (int i = 0; i < 8; i++) {
            printf("%2d|", i+1);
            for (int j = 7; j >= 0; j--) {
                if (tabuleiro[i][j] == ESPACO_VAZIO) {
                    char c = ((j%2) ^ (i%2)) ? '.': ' ';
                    printf("  %c", c);
                } else {
                    char c = espaco_para_char(tabuleiro[i][j]);
                    printf(" %d%c", (tabuleiro[i][j] < 6)? 1: 2, c);
                }
            }
            printf("|\n");

            printf("  |");
            for (int j = 0; j < 8; j++) {
                printf("   ");
            }
            printf("|\n");
        }
    } else { // primeiro jogador
        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("  %c", 'A' + i);
        }
        putchar('\n');

        printf("   ");
        for (int i = 0; i < 8; i++) {
            printf("___");
        }
        putchar('\n');

        for (int i = 7; i >= 0; i--) {
            printf("%2d|", i+1);
            for (int j = 0; j < 8; j++) {
                if (tabuleiro[i][j] == ESPACO_VAZIO) {
                    char c = (j%2) ^ (i%2) ? '.': ' ';
                    printf("  %c", c);
                } else {
                    char c = espaco_para_char(tabuleiro[i][j]);
                    printf(" %d%c", (tabuleiro[i][j] < 6)? 1: 2, c);
                }
            }
            printf("|\n");

            printf("  |");
            for (int j = 0; j < 8; j++) {
                printf("   ");
            }
            printf("|\n");
        }
    }
}
#endif //ifdef MODO_UTF8