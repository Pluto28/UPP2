#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

enum { NADA, USER, BONUS, OBSTACULO };

struct UserData {
  int x;
  int y;
  int score;
  int vidas;
};

// int is_bonus(user_pos, map);
void imprimeMapa(int mapa[10][10], WINDOW *tela);

// Popula a struct e o mapa com os dados lidos do arquivo
void popula_dados(struct UserData *user, int mapa[10][10]);

// Entra em um modo especial do terminal que permite ler caracteres
// iterativamente e imprimir coisas como se o terminal fosse uma tela
// com coordenadas. Olhe as funções mvwaddstr, wclear, mvwaddch,
// wgetch, etc para como fazer coisas neste modo.
WINDOW *terminal_raw();

// Sai do modo especial do terminal.
void terminal_noraw();

// coloca o score do usuário no buffer da tela
void print_score(WINDOW *tela, int score);

// Loop do jogo, executa até que o usuário aperte alguma tecla específica
void playing_loop(struct UserData *user, int mapa[10][10]);

// Atualiza a interface gráfica que o usuário vê
void atualiza_tela(WINDOW *tela, struct UserData *user, int mapa[10][10]);

// Se bônus tem valor 1, a quantidade de pontos é x.
// Se bônus tem valor 2, a quantidade de pontos é 2x
void update_score(struct UserData *user, int bonus_val);

// INCLUA EM SEU CODIGO AS FUNCOES randomInteger, geraMapa E atualizaMapa
// ATENCAO: NAO ALTERE NENHUMA DESSAS TRES FUNCOES

int randomInteger(int low, int high) {
  int k;
  double d;

  d = (double)rand() / ((double)RAND_MAX + 1);
  k = d * (high - low + 1);

  return low + k;
}

void geraMapa(int mapa[10][10], int nivel) {
  int i, j, qtdO, qtdA, lin, col, infO = 5, supO = 15;

  switch (nivel) {
  case 2:
    infO = 5;
    supO = 25;
    break;
  case 3:
    infO = 15;
    supO = 35;
    break;
  }

  srand((unsigned int)time(NULL));

  for (i = 0; i < 10; i++)
    for (j = 0; j < 10; j++)
      mapa[i][j] = 0;

  // Usuário representado por 1
  mapa[randomInteger(0, 9)][0] = 1;

  // Objetivo representado por 2
  mapa[randomInteger(0, 9)][9] = 2;

  // Escolhe densidade dos bônus
  qtdA = randomInteger(3, 10);

  // Escolhe densidade dos obstáculos
  qtdO = randomInteger(infO, supO);

  // Popula o mapa de acordo com as densidades aleatoriamente selecionadas
  // posteriormente
  while (qtdO > 0) {
    lin = randomInteger(0, 9);
    col = randomInteger(1, 8);
    if (mapa[lin][col] == 0) {
      mapa[lin][col] = 3;
      qtdO--;
    }
  }

  while (qtdA > 0) {
    lin = randomInteger(0, 9);
    col = randomInteger(1, 8);
    if (mapa[lin][col] == 0) {
      mapa[lin][col] = 4;
      qtdA--;
    }
  }
}

// mudar isso aqui? Nop
void atualizaMapa(int mapa[10][10]) {
  int i, j, upd, atualizado[10][10];
  for (i = 0; i < 10; i++)
    for (j = 0; j < 10; j++)
      atualizado[i][j] = 0;

  for (i = 0; i < 10; i++)
    for (j = 0; j < 10; j++) {
      if ((mapa[i][j] == 2 || mapa[i][j] == 3 || mapa[i][j] == 4) &&
          atualizado[i][j] == 0) {

        // upd vai de 1 até 4, cada coisa provendo uma funcionalidade
        // 1:
        //     - move o objeto 1 linha para baixo da posição inicial
        // 2:
        //     - move o objeto 1 linha para cima da posição inicial
        // 3:
        //     - move o objeto 1 coluna para a direita da posição inicial
        // 4:
        //     - move o objeto 1 linha para a esquerda da posição inicial
        // OBS:
        //     Nunca deixa qualquer objeto que não seja o usuário ou o
        //      objetivo entrar nas colunas 0 ou 9 do mapa
        upd = randomInteger(1, 4);
        switch (upd) {
        case 1:
          if ((mapa[i - 1][j] == 0 ||
               (mapa[i - 1][j] == 1 && mapa[i][j] == 3)) &&
              i > 0) {
            mapa[i - 1][j] = mapa[i][j];
            mapa[i][j] = 0;
            atualizado[i - 1][j] = 1;
          }
          break;
        case 2:
          if ((mapa[i + 1][j] == 0 ||
               (mapa[i + 1][j] == 1 && mapa[i][j] == 3)) &&
              i < 9) {
            mapa[i + 1][j] = mapa[i][j];
            mapa[i][j] = 0;
            atualizado[i + 1][j] = 1;
          }
          break;
        case 3:
          if ((mapa[i][j + 1] == 0 ||
               (mapa[i][j + 1] == 1 && mapa[i][j] == 3)) &&
              j < 9) {
            mapa[i][j + 1] = mapa[i][j];
            mapa[i][j] = 0;
            atualizado[i][j + 1] = 1;
          }
          break;
        case 4:
          if ((mapa[i][j - 1] == 0 ||
               (mapa[i][j - 1] == 1 && mapa[i][j] == 3)) &&
              j > 0) {
            mapa[i][j - 1] = mapa[i][j];
            mapa[i][j] = 0;
            atualizado[i][j - 1] = 1;
          }
        }
      }
    }
}

void terminal_noraw() {
  echo();
  nocbreak();
  endwin();
}

WINDOW *terminal_raw() {
  WINDOW *tela = initscr();
  cbreak();
  noecho();
  return tela;
}

void imprimeScore(WINDOW *tela, int score) {
  mvwprintw(tela, 0, 10, "%i", score);
  wrefresh(tela);
}

// @TODO: write this
void imprimeMapa(int mapa[10][10], WINDOW *tela) {
  int col, row;

  for (row = 0; row < 10; row++) {
    for (col = 0; col < 10; col++) {
      switch (mapa[row][col]) {
      case 1:
        mvwaddstr(tela, 5 + row, col * 2, "~~");
        break;
      case 2:
        mvwaddstr(tela, 5 + row, col * 2, "VV");
        break;
      case 3:
        mvwaddstr(tela, 5 + row, col * 2, "BB");
        break;
      case 4:
        mvwaddstr(tela, 5 + row, col * 2, "BB");
        break;
      default:
        mvwaddstr(tela, 5 + row, col * 2, "..");
        break;
      }
      waddch(tela, '\n');
    }
  }
}

void atualiza_tela(WINDOW *tela, struct UserData *user, int mapa[10][10]) {
  wclear(tela);
  imprimeMapa(mapa, tela);
  imprimeScore(tela, user->score);
  wrefresh(tela);
}

void playing_loop(struct UserData *user, int mapa[10][10]) {
  int ch = '\0';
  WINDOW *tela = terminal_raw();

  while (ch != 'q') {
    ch = wgetch(tela);
    switch (ch) {
    case 'w':
      mvwaddstr(tela, 20, 0, "u");
      atualizaMapa(mapa);
      atualiza_tela(tela, user, mapa);
      mvwaddch(tela, 20, 2, ch);
      break;
    case 'a':
      mvwaddstr(tela, 20, 0, "l");
      atualizaMapa(mapa);
      atualiza_tela(tela, user, mapa);
      mvwaddch(tela, 20, 2, ch);
      break;
    case 's':
      mvwaddstr(tela, 20, 0, "d");
      atualizaMapa(mapa);
      atualiza_tela(tela, user, mapa);
      mvwaddch(tela, 20, 2, ch);
      break;
    case 'd':
      mvwaddstr(tela, 20, 0, "r");
      atualizaMapa(mapa);
      atualiza_tela(tela, user, mapa);
      mvwaddch(tela, 20, 2, ch);
      break;
    }
  }

 terminal_noraw();
}

int main() {
  setlocale(LC_ALL, "Portuguese");
  int mapa[10][10], at;
  struct UserData user = {0, 0, 10, 3};

  // TODA VEZ QUE VOCE DESEJAR GERAR UM NOVO MAPA, BASTA CHAMAR A FUNCAO
  // geraMapa, conforme o exemplo a seguir O primeiro par�metro � a sua matriz
  // 10x10, o segundo � o n�vel de dificuldade do jogo: 1-F�cil,
  // 2-Intermedi�rio, 3-Dif�cil
  geraMapa(mapa, 1); // AQUI ESTOU USANDO O N�VEL DE DIFICULDADE 3 - DIF�CIL

  do {
    printf("\nAtualizar mapa? Resposta (1-Sim, 0-N�o): ");
    scanf("%d", &at);

    // TODA VEZ QUE VOCE DESEJAR ATUALIZAR UM MAPA, BASTA CHAMAR A FUNCAO
    // atualizaMapa, conforme o exemplo a seguir
    if (at) {
      playing_loop(&user, mapa);
    }

  } while (at);

  return 0;
}
