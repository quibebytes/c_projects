// Regras baseadas na versao japonesa/coreana,
// porque nao queria implementar superko.
//
// Sou um completo noob em Go entao talvez haja
// algumas decisoes mal pensadas no algoritmo
//
// TODO transformar os algoritmos que utilizam a
// pilha para usar um tabuleiro de bools.
//
// Nao implementei seki :-P
#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define TABULEIRO_LARGURA 9
#define TABULEIRO_ALTURA 9
#define VALOR_KOMI 6.5f
#define DENTRO_DO_TABULEIRO(x, y) ((x) >= 0 && (x) < TABULEIRO_LARGURA && (y) >= 0 && (y) < TABULEIRO_ALTURA)

typedef enum Espaco {
   ESPACO_VAZIO, 
   ESPACO_1,
   ESPACO_2
} Espaco;

typedef struct Posicao {
    int x;
    int y;
} Posicao;

static const char pedras_chars[] = {' ', 'X', 'O'};
const static Posicao cruz[4] = {
        { .x = -1, .y = 0 },
        { .x = 1, .y = 0 },
        { .x = 0, .y = -1 },
        { .x = 0, .y = 1 }
};

static Espaco tabuleiro[TABULEIRO_ALTURA][TABULEIRO_LARGURA];
static Posicao pilha_corrente[TABULEIRO_LARGURA * TABULEIRO_ALTURA];
static int pilha_qtd_items = 0;
static int vezes_passadas = 0;


void imprimir_tabuleiro(bool jogador);
void conseguir_coordenadas(char *entrada, int bufsiz, Posicao *posicao);
bool posicao_eh_valida(bool jogador, Posicao posicao);
bool pedra_esta_livre(Posicao posicao, bool jogador);
void remover_pedras(bool jogador);
void contar_territorio(int *territorio_preto, int *territorio_branco, int *territorio_aberto);


uint64_t fnv_1a(uint8_t *data, uint64_t n_de_bytes) {
    uint64_t hash = 0xcbf29ce484222325;

    for (uint64_t i = 0; i < n_de_bytes; i++) {
        hash ^= data[i];
        hash *= 0x00000100000001b3;
    }

    return hash;
}


// TODO structs Posicao, e Posicao?

int main() {
    char entrada[BUFSIZ];
    bool loop = true, jogador = false;
    Posicao posicao = { .x = 0, .y = 0 };
    Posicao ultima_posicao = { .x = 0, .y = 0 };
    uint64_t hash_ultima_jogada = 0;
    uint64_t hash_penultima_jogada = 0;


    const char msg_vazia[] = "";
    const char msg_ocupada[] = "Posicao nao esta vazia.\nTente novamente";
    const char msg_passe[] = "Vez passada";
    const char msg_suicidio[] = "Jogada feita foi um suicidio!\nSe voce quise passar a vez digite \"passar\"";
    const char msg_ko[] = "Jogada feita foi ko!\nTente outra jogada";
    const char *msg_atual = msg_vazia;
 
    for (int i = 0; i < TABULEIRO_ALTURA; i++) {
        for (int j = 0; j < TABULEIRO_LARGURA; j++) {
            tabuleiro[i][j] = ESPACO_VAZIO;
        }
    }

    puts("Go - Digite \"ajuda\" para tirar duvidas");

    do {
        imprimir_tabuleiro(jogador);
        printf("Vez do %s\n", jogador? "O (branco)": "X (preto)");
        puts(msg_atual);
        if (vezes_passadas == 0) { ultima_posicao = posicao; }
        conseguir_coordenadas(entrada, BUFSIZ, &posicao);
        if (vezes_passadas > 1) { break; }

        if (vezes_passadas == 0 && posicao_eh_valida(jogador, posicao)) {
            tabuleiro[posicao.y][posicao.x] = jogador? ESPACO_2: ESPACO_1;
            
            bool posicao_esta_livre = pedra_esta_livre(posicao, jogador);
            pilha_qtd_items = 0;
            bool tem_inimigo_capturado = false;

            // Talvez essa forma nao lide com todos os casos de suicidio.
            // se isso atrapalhar algum jogador isso eh pq estamos utilizando
            // um subconjunto de regras super especifico para um jogo mais balanceado tm
            for (int i = 0; !posicao_esta_livre && i < 4 && !tem_inimigo_capturado; i++) {
                Posicao vizinho = { .x = posicao.x + cruz[i].x, .y = posicao.y + cruz[i].y };
                if (DENTRO_DO_TABULEIRO(vizinho.x, vizinho.y) && tabuleiro[vizinho.y][vizinho.x] == (jogador? ESPACO_1: ESPACO_2)) {
                    tem_inimigo_capturado = !pedra_esta_livre(vizinho, !jogador);
                    pilha_qtd_items = 0;
                }
            }

            if (posicao_esta_livre || tem_inimigo_capturado) {
                remover_pedras(!jogador);
                remover_pedras(jogador);
                
                uint64_t hash_atual = fnv_1a((uint8_t*) tabuleiro, sizeof(Espaco) * TABULEIRO_ALTURA * TABULEIRO_LARGURA);

                if (hash_atual != hash_ultima_jogada && hash_atual != hash_penultima_jogada) {
                    hash_penultima_jogada = hash_ultima_jogada;
                    hash_ultima_jogada = hash_atual;
                    msg_atual = msg_vazia;
                    jogador = !jogador;
                } else {
                    if (hash_atual == hash_ultima_jogada) {
                        msg_atual = msg_suicidio;
                    }

                    if (hash_atual == hash_penultima_jogada) {
                        tabuleiro[ultima_posicao.y][ultima_posicao.x] = jogador? ESPACO_1: ESPACO_2;
                        remover_pedras(jogador);
                        remover_pedras(!jogador);
                        msg_atual = msg_ko;
                    }
                }
            } else {
                tabuleiro[posicao.y][posicao.x] = ESPACO_VAZIO;
                msg_atual = msg_suicidio;
            }
        } else if (vezes_passadas == 1) {
            msg_atual = msg_passe;
            jogador = !jogador;
        } else {
            msg_atual = msg_ocupada;
        }
    } while (loop);

    puts("Finalizando partida...");
    int territorio_preto = 0, territorio_branco = 0, territorio_aberto = 0;
    contar_territorio(&territorio_preto, &territorio_branco, &territorio_aberto);

    puts("\n  PRETO   BRANCO   ABERTO  ");
    printf("| %5d | %6d | %6d | TERRITORIO\n", territorio_preto, territorio_branco, territorio_aberto);
    printf("| %5d | %6.1f | %6d | KOMI\n", 0, VALOR_KOMI, 0);
    printf("| %5d | %6.1f |        | PONTUACAO\n\n", territorio_preto, (float) territorio_branco + VALOR_KOMI);

    if ((float) territorio_preto > ((float) territorio_branco + VALOR_KOMI)) {
        puts("Vitoria do jogador preto (X)!");
    } else if ((float) territorio_preto < ((float) territorio_branco + VALOR_KOMI)) {
        puts("Vitoria do jogador branco (O)!");
    } else {
        puts("Empate!");
    }

    puts("Ainda considerando posicoes em seki");

    return EXIT_SUCCESS;
}

void conseguir_coordenadas(char *entrada, int bufsiz, Posicao *posicao) {
    bool sucesso = false;

    do {
        printf("# ");
        char *resultado = fgets(entrada, bufsiz, stdin);

        if (feof(stdin)) { puts("^D"); exit(EXIT_SUCCESS); }

        for (int i = 0; (i < bufsiz) && resultado; i++) {
            if (resultado[i] == '\n') {
                resultado[i] = '\0';
            }
        }


        if (!strcmp(entrada, "ajuda")) {
            puts("\nDigite as coordenadas na forma \"<letra> <digito>\", exemplo: d4.\n");
            puts("Se voce digitar uma posicao invalida, o jogo vai reiniciar seu turno,");
            puts("entao se atente de quem eh a vez na mensagem abaixo do tabuleiro!\n");
            puts("X = preto (primeiro), O = branco (segundo)\n");
        } else if (!strcmp(entrada, "passar")) {
            vezes_passadas++;
            sucesso = true;
       } else {
            char c_x;
            sucesso = sscanf(entrada, "%c %d", &c_x, &posicao->y) == 2;
            posicao->x = ((c_x > 0x60 && c_x <= 0x7A)? c_x - 0x20: c_x) - 0x40;

            posicao->x -= 1;
            posicao->y -= 1;

            if (!sucesso || !DENTRO_DO_TABULEIRO(posicao->x, posicao->y)) {
                puts("Erro ao ler coordenadas, tente de novo.");
                puts("Se estiver com duvidas, digite \"ajuda\".");
                sucesso = false;
            } else {
                vezes_passadas = 0;
            }
        }
    } while (!sucesso);
}

void imprimir_tabuleiro(bool jogador) {
    int inc = jogador? -1: 1; // true = 2 = O = branco
    int comeco_i = jogador? 0: TABULEIRO_ALTURA-1;
    int comeco_j = jogador? TABULEIRO_LARGURA-1: 0;
    int final_i = jogador? TABULEIRO_ALTURA: -1;
    int final_j = jogador? 0: TABULEIRO_LARGURA-1;

    for (int i = 0; i < TABULEIRO_LARGURA; i++) {
        printf("%c ", jogador? (('@' + TABULEIRO_LARGURA) - i): ('A' + i));
    }

    putchar('\n');

    for (int i = 0; i < TABULEIRO_LARGURA-1; i++) {
        printf("__");
    }

    printf("_\n");
    
    for (int i = comeco_i; i != final_i; i -= inc) {
        for (int j = comeco_j; j != final_j; j += inc) {
            printf("%c ", pedras_chars[tabuleiro[i][j]]);
        }

        printf("%c| %d\n", pedras_chars[tabuleiro[i][jogador? 0: TABULEIRO_LARGURA-1]], i+1);
    }
}


// regra 7A da wikipedia, vou deixar pra depois
bool posicao_eh_valida(bool jogador, Posicao posicao) {
    return DENTRO_DO_TABULEIRO(posicao.x, posicao.y) && tabuleiro[posicao.y][posicao.x] == ESPACO_VAZIO;
}


// Assume que a posicao no argumento pertence ao jogador correto
// Assume que a posicao esta dentro do tabuleiro
// Tambem assume que a pilha eh zerada por outras funcoes
// Utiliza posicoes diretas do tabuleiro ([0;TABULEIRO_LARGURA[, etc)
bool pedra_esta_livre(Posicao posicao, bool jogador) { 

    if (tabuleiro[posicao.y][posicao.x] == (jogador? ESPACO_2: ESPACO_1)) {
        pilha_corrente[pilha_qtd_items] = posicao;
        pilha_qtd_items++;
    }

    bool livre = false;

    for (int i = 0; i < 4 && !livre; i++) {
        const Posicao vizinho = { .x = posicao.x + cruz[i].x, .y = posicao.y + cruz[i].y };

        if (DENTRO_DO_TABULEIRO(vizinho.x, vizinho.y)) {
            if (tabuleiro[vizinho.y][vizinho.x] == ESPACO_VAZIO) {
                livre = true;
            } else if (tabuleiro[vizinho.y][vizinho.x] == (jogador? ESPACO_2: ESPACO_1)) {
                bool ja_caminhado = false;
                for (int k = 0; k < pilha_qtd_items && !ja_caminhado; k++) {
                    if (pilha_corrente[k].x == vizinho.x && pilha_corrente[k].y == vizinho.y) {
                        ja_caminhado = true;
                    }
                }

                if (!ja_caminhado) {
                    livre = livre || pedra_esta_livre(vizinho, jogador);
                }
            }
        }
    }

    return livre;
}

void remover_pedras(bool jogador) {
    for (int i = 0; i < TABULEIRO_ALTURA; i++) {
        for (int j = 0; j < TABULEIRO_LARGURA; j++) {

            if (
                tabuleiro[i][j] == (jogador? ESPACO_2: ESPACO_1) &&
                !pedra_esta_livre((Posicao) { .x = j, .y = i }, jogador)
            ) {
                for (int k = 0; k < pilha_qtd_items; k++) {
                    tabuleiro[pilha_corrente[k].y][pilha_corrente[k].x] = ESPACO_VAZIO;
                }
            }

            pilha_qtd_items = 0;
        }
    }
}

// TODO similar dms a pedra_esta_livre?
bool posicao_pertence_a_territorio(Posicao posicao, bool jogador) {
    bool pertence = true;

    pilha_corrente[pilha_qtd_items] = posicao;
    pilha_qtd_items++;

    for (int i = 0; i < 4 && pertence; i++) {
        Posicao vizinho = { .x = posicao.x + cruz[i].x, .y = posicao.y + cruz[i].y };

        if (DENTRO_DO_TABULEIRO(vizinho.x, vizinho.y)) {
            if (tabuleiro[vizinho.y][vizinho.x] == (jogador? ESPACO_1: ESPACO_2)) {
                pertence = false;
            } else if (tabuleiro[vizinho.y][vizinho.x] == ESPACO_VAZIO) {
                bool ja_caminhado = false;
                for (int k = 0; k < pilha_qtd_items && !ja_caminhado; k++) {
                    if (pilha_corrente[k].x == vizinho.x && pilha_corrente[k].y == vizinho.y) {
                        ja_caminhado = true;
                    }
                }

                if (!ja_caminhado) {
                    pertence = pertence && posicao_pertence_a_territorio(vizinho, jogador);
                }
            }
        }
    }

    return pertence;
}

void contar_territorio(int *territorio_preto, int *territorio_branco, int *territorio_aberto) {
    *territorio_preto = 0;
    *territorio_branco = 0;
    *territorio_aberto = 0;

    for (int i = 0; i < TABULEIRO_ALTURA; i++) {
        for (int j = 0; j < TABULEIRO_LARGURA; j++) {
            Posicao posicao = { .x = j, .y = i };
            if (tabuleiro[i][j] == ESPACO_VAZIO) {
                bool pertence_a_preto = posicao_pertence_a_territorio(posicao, false);
                pilha_qtd_items = 0;
                bool pertence_a_branco = posicao_pertence_a_territorio(posicao, true);
                pilha_qtd_items = 0;

                *territorio_preto += pertence_a_preto;
                *territorio_branco += pertence_a_branco;
                *territorio_aberto += (!pertence_a_preto && !pertence_a_branco);
            }
        }
    }
}
