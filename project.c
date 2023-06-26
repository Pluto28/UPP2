#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct UserData {
  int x;
  int y;
  int score;
  int vidas;
};

void imprimeMapa(int mapa[10][10], WINDOW *tela);

// Popula a struct e o mapa com os dados lidos do arquivo
// TODO
void popula_dados(struct UserData *user, int mapa[10][10]);

// Entra em um modo especial do terminal que permite ler caracteres
// iterativamente e imprimir coisas como se o terminal fosse uma tela
// com coordenadas. Olhe as funções mvwaddstr, wclear, mvwaddch,
// wgetch, etc para como fazer coisas neste modo.
// DONE
WINDOW *terminal_raw();

// Sai do modo especial do terminal.
// DONE
void terminal_noraw();

// coloca o score do usuário no buffer da tela
// DONE
void info_print(WINDOW *tela, struct UserData *user);

// Loop do jogo, executa até que o usuário aperte alguma tecla específica
// DONE
void playing_loop(struct UserData *user, int mapa[10][10]);

// Atualiza a interface gráfica que o usuário vê
// DONE
void atualiza_tela(WINDOW *tela, struct UserData *user, int mapa[10][10]);

// Se bônus tem valor 1, a quantidade de pontos é x.
// Se bônus tem valor 2, a quantidade de pontos é 2x
// DONE
void update_score(struct UserData *user, int bonus_type);

// Checa se a célula do mapa para a qual o usuário vai se mover contém
// o objeto que garante vitória
// DONE
int is_victory(struct UserData *user, int mapa[10][10]);

// modifica a estrutura de dados do usuário de acordo com o evento vitória
// HALF-DONE
void victory(WINDOW *screen, struct UserData *user);

// Checa se a célula para qual o usuário está se movendo contém um bônus
// DONE 
// int is_bonus(struct UserData *user, int mapa[10][10]);
int confereResposta(int n);

// Performa todas as checagens necessárias antes de mover o usuário.
//   - Checa se o usuário ganhará a partida.
//   - Checa se o usuário passará por um bônus
//   - checa se o usuário passará por um obstáculo
// Em cada caso, os dados do usuário são modificados de acordo
//
// NOTA: ESTA FUNÇÃO DEVE SER CHAMADA ANTES DE SE FAZER O UPDATE DO USUÁRIO
// NO MAPA PORQUE O UPDATE OCASIONA PERDA DE DADOS
//
// TODO
int check_move(int new_x, int new_y, int mapa[10][10], struct UserData *user);

int movePersonagem(int mapa[10][10], char mover);

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

void info_print(WINDOW *tela, struct UserData *user) {
  mvwprintw(tela, 0, 0, "Score: %i     Vidas: %i", user->score, user->vidas);
}

int confereResposta(int n) {
  while (n != 1 && n != 2) {
    printf("Digite apenas 1 ou 2!\n");
    scanf("%i", &n);
  }
  return n + 1;
}

int movePersonagem(int mapa[10][10], char mover) {
  char w, s, a, d;
  int i, j;

  switch (mover) {
  case 'w':
    for (i = 0; i < 10; i++) {
      for (j = 0; j < 10; j++) {
        if (mapa[i][j] == 1) {
          mapa[i][j] = 0;

          mapa[i - 1][j] = 1;
          atualizaMapa(mapa);
          return 1;
        }
      }
    }

  case 's':
    for (i = 0; i < 10; i++) {
      for (j = 0; j < 10; j++) {
        if (mapa[i][j] == 1) {
          mapa[i][j] = 0;
          mapa[i + 1][j] = 1;
          atualizaMapa(mapa);
          return 1;
        }
      }
    }

  case 'a':
    for (i = 0; i < 10; i++) {
      for (j = 0; j < 10; j++) {
        if (mapa[i][j] == 1) {
          mapa[i][j] = 0;
          mapa[i][j - 1] = 1;
          atualizaMapa(mapa);
          return 1;
        }
      }
    }

  case 'd':
    for (i = 0; i < 10; i++) {
      for (j = 0; j < 10; j++) {
        if (mapa[i][j] == 1) {
          mapa[i][j] = 0;
          mapa[i][j + 1] = 1;
          atualizaMapa(mapa);
          return 1;
        }
      }
    }
  }
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
        mvwaddstr(tela, 5 + row, col * 2, "OO");
        break;
      case 4:
        mvwaddstr(tela, 5 + row, col * 2, "BB");
        break;
      default:
        mvwaddstr(tela, 5 + row, col * 2, "..");
        break;
      }
    }
  }
}

void atualiza_tela(WINDOW *tela, struct UserData *user, int mapa[10][10]) {
  // primeiro limpamos o buffer armazenando o estado da tela
  wclear(tela);

  // Preenchemos a estrutura de dados que
  // armazena o estado da tela com o estado atual do mapa
  imprimeMapa(mapa, tela);

  // Mesma coisa que para Mapa, mas agora o score do usuário
  info_print(tela, user);

  // Faz o update da tela que o usuário vê usando os dados armazenados no 
  // buffer
  wrefresh(tela);
}

int is_victory(struct UserData *user, int mapa[10][10]) {
  if (mapa[user->y][user->x] == 2) {
    return 1;
  }

  return 0;
}

void victory(WINDOW *screen, struct UserData *user) {
  update_score(user, 2);

  wclear(screen);
  mvwaddstr(screen, 10, 0, "| | | (_) |           (_)      ");
  mvwaddstr(screen, 11, 0, "| | | |_| |_ ___  _ __ _  __ _ ");
  mvwaddstr(screen, 12, 0, "| | | | | __/ _ \\| '__| |/ _` |");
  mvwaddstr(screen, 13, 0, "\\ \\_/ / | || (_) | |  | | (_| |");
  mvwaddstr(screen, 14, 0, " \\___/|_|\\__\\___/|_|  |_|\\__,_|");
  wrefresh(screen);

  mvwaddstr(screen, 20, 0, "Digite qualquer caractere para sair");
  wgetch(screen);
}

void update_score(struct UserData *user, int multiplier) {
  // TODO: Create constant defining a base amount of points,
  // the multiplier changing it
  user->score = user->score + (multiplier * 20);
}

int is_bonus(struct UserData *user, int mapa[10][10]) {
  if (mapa[user->y][user->x] == 4) {
    return 1;
  }
  return 0;
}

void playing_loop(struct UserData *user, int mapa[10][10]) {
  int ch = '\0';
  int prev_x, prev_y;
  WINDOW *tela = terminal_raw();
  popula_dados(user, mapa);

  while (ch != 'q') {
    atualiza_tela(tela, user, mapa);
    // atualizaPosicaoMapa(mapa);
    atualizaMapa(mapa);

    // Obstáculos podem se mover na direção do usuário, um cheque é necessário
    // assim que o mapa é atualizado


    // Lê entrada de usuário no modo especial
    ch = wgetch(tela);
    // atualizaPosicao(user);
    // movePersonagem(mapa, ch);
    switch (ch) {
    case 'w':
      mapa[user->y][user->x] = 0;
      user->y -= 1;
      break;
    case 'a':
      mapa[user->y][user->x] = 0;
      user->x -= 1;
      break;
    case 's':
      mapa[user->y][user->x] = 0;
      user->y += 1;
      break;
    case 'd':
      mapa[user->y][user->x] = 0;
      user->x += 1;
      break;
    default:
      continue;
    }

    // Caso o usuário esteja se movendo para uma célula que contém um objeto que 
    // causa um evento, precisamos detectar
    if (is_victory(user, mapa)) {
      victory(tela, user);
      break;
    } else if (is_bonus(user, mapa)) {
      update_score(user, 1);
    } // TODO: Function for obstacles

    mapa[user->y][user->x] = 1;

  }

  terminal_noraw();
}

void popula_dados(struct UserData *user, int mapa[10][10]) {
  int x, y;
  for (y = 0; y < 10; ++y) {
    if (mapa[y][0] == 1) {

      user->x = 0;
      user->y = y;
    }
  }

  user->score = 0;
}

int main() {
  setlocale(LC_ALL, "Portuguese");
  int mapa[10][10], at;
  struct UserData user;

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
