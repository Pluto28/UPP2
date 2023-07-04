#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PRETO "\e[0;30m"
#define VERMELHO "\e[0;31m"
#define VERDE "\e[0;32m"
#define AMARELO "\e[0;33m"
#define AZUL "\e[0;34m"
#define MAGENTA "\e[0;35m"
#define CIANO "\e[0;36m"
#define BRANCO "\e[0;37m"
#define RESET "\e[0m"

struct UserData {
  int x;
  int y;
  int score;
  int vidas;
  char nome[30];
  int resposta;
};

void atualizaRankingDados(struct UserData *user, char nomes[3][30],
                          int scores[3]);

void confereNome(char nome[30]) {
  if (strlen(nome) > 30) {
    while (strlen(nome) > 30) {
      printf("O seu nome deve conter no máximo 30 caracteres! Digite outro "
             "nome:\n");
      scanf("%s", nome);
    }
  }
}

void imprimeMapa(int mapa[10][10], WINDOW *tela);

// Se bônus tem valor 1, a quantidade de pontos é x.
// Se bônus tem valor 2, a quantidade de pontos é 2x
// DONE
void update_score(struct UserData *user, int multiplier);

void leRanking(char nomes[3][30], int pontos[3]);

void atualizaRanking(char nomes[3][30], int pontos[3]);

// Popula a struct e o mapa com os dados lidos do arquivo
// TODO
void popula_dados(struct UserData *user, int mapa[10][10]);

void menuPrincipal();

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
void game_loop(struct UserData *user, int mapa[10][10]);

// Atualiza a interface gráfica que o usuário vê
// DONE
void atualiza_tela(WINDOW *tela, struct UserData *user, int mapa[10][10]);

// Checa se a célula do mapa para a qual o usuário vai se mover contém
// o objeto que garante vitória
// DONE
int is_victory(struct UserData *user, int mapa[10][10]);

// modifica a estrutura de dados do usuário de acordo com o evento vitória
// DONE
void victory(WINDOW *screen, struct UserData *user);

// Checa se a célula para qual o usuário está se movendo contém um bônus
// DONE
int is_bonus(struct UserData *user, int mapa[10][10]);

int confereResposta(int n);

int movePersonagem(int mapa[10][10], char mover);

int is_obstaculo(struct UserData *user, int mapa[10][10]);

int is_limitedomapa(struct UserData *user, int mapa[10][10]);

void leRanking(char nomes[3][30], int pontos[3]) {
  FILE *arq = NULL;
  int res;
  arq = fopen("rank.dat", "rb");
  if (arq != NULL) {
    printf("\nLendo os dados ...");
    res = fread(nomes, sizeof(char), 90, arq);
    if (res == 90) {
      printf("\nNomes lidos corretamente");
    }
    res = fread(pontos, sizeof(int), 3, arq);
    if (res == 3) {
      printf("\nPontos lidos corretamente");
    }
    fclose(arq);
  } else {
    printf("\n\nNa primeira vez não há nada a ser lido.");
  }
}

void atualizaRanking(char nomes[3][30], int pontos[3]) {
  FILE *arq = NULL;
  int res;
  arq = fopen("rank.dat", "wb");
  if (arq != NULL) {
    printf("\nComeçando a gravação ...");
    res = fwrite(nomes, sizeof(char), 90, arq);
    if (res == 90) {
      printf("\nNomes gravados corretamente");
    }
    res = fwrite(pontos, sizeof(int), 3, arq);
    if (res == 3) {
      printf("\nPontos gravados corretamente");
    }
    fclose(arq);
  } else {
    printf("\nProblemas na gravação.");
  }
}

int is_bonus(struct UserData *user, int mapa[10][10]) {
  int x = user->x;
  int y = user->y;

  if (mapa[user->y][user->x] == 4) {
    return 1;
  }

  return 0;
}

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
        // - move o objeto 1 linha para baixo da posição inicial
        // 2:
        // - move o objeto 1 linha para cima da posição inicial
        // 3:
        // - move o objeto 1 coluna para a direita da posição inicial
        // 4:
        // - move o objeto 1 linha para a esquerda da posição inicial
        // OBS:
        // Nunca deixa qualquer objeto que não seja o usuário ou o
        // objetivo entrar nas colunas 0 ou 9 do mapa
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
  mvwprintw(tela, 0, 0, "Crunopios: %i Vidas: %i", user->score, user->vidas);
  mvwprintw(tela, 1, 0, "Sair da partida atual(b).");
}

int confereResposta(int n) {
  while (n != 1 && n != 2) {
    printf("Digite apenas 1 ou 2!\n");
    setbuf(stdin, NULL);
    scanf("%i", &n);
  }
  return n;
}

// @TODO: write this
void imprimeMapa(int mapa[10][10], WINDOW *tela) {
  int col, row;
  int col_const_stretch = 4;
  int row_const_stretch = 2;

  int row_offset = 2;
  int col_offset = 0;

  for (row = 0; row < 10; row++) {
    for (col = 0; col < 10; col++) {
      switch (mapa[row][col]) {
      case 1:
        mvwaddstr(tela, (row * row_const_stretch) + row_offset,
                  (col * col_const_stretch) + col, "))))");
        mvwaddstr(tela, (row * row_const_stretch) + row_offset + 1,
                  (col * col_const_stretch) + col, "))))");
        break;
      case 2:
        mvwaddstr(tela, (row * row_const_stretch) + row_offset,
                  (col * col_const_stretch) + col, "VVVV");
        mvwaddstr(tela, (row * row_const_stretch) + row_offset + 1,
                  (col * col_const_stretch) + col, "VVVV");
        break;
      case 3:
        mvwaddstr(tela, (row * row_const_stretch) + row_offset,
                  (col * col_const_stretch) + col, "OOOO");
        mvwaddstr(tela, (row * row_const_stretch) + row_offset + 1,
                  (col * col_const_stretch) + col, "OOOO");
        break;
      case 4:
        mvwaddstr(tela, (row * row_const_stretch) + row_offset,
                  (col * col_const_stretch) + col, "BBBB");
        mvwaddstr(tela, (row * row_const_stretch) + row_offset + 1,
                  (col * col_const_stretch) + col, "BBBB");
        break;
      default:
        mvwaddstr(tela, (row * row_const_stretch) + row_offset,
                  (col * col_const_stretch) + col, "....");
        mvwaddstr(tela, (row * row_const_stretch) + row_offset + 1,
                  (col * col_const_stretch) + col, "....");

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
  mvwprintw(screen, 16, 0,
            "Parabéns %s, você chegou ao planeta V são e salvo(não tão são) e "
            "com %i crunopios para gastar como desejar.",
            user->nome, user->score);
  wrefresh(screen);

  mvwaddstr(screen, 20, 0, "Digite qualquer caractere para sair");
  wgetch(screen);
}

void update_score(struct UserData *user, int multiplier) {
  // TODO: Create constant defining a base amount of points,
  // the multiplier changing it
  user->score = user->score + (multiplier * 20);
}

void game_loop(struct UserData *user, int mapa[10][10]) {
  int ch = '\0';
  int prev_x, prev_y;
  WINDOW *tela = terminal_raw();
  popula_dados(user, mapa);
  int resposta;

  atualiza_tela(tela, user, mapa);

  while (ch != 'b') {
    // Lê entrada de usuário no modo especial
    flushinp(); // parecido com setbuf(stdin, NULL)
    ch = wgetch(tela);
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

    // Caso o usuário esteja se movendo para uma célula contendo um objeto
    // que causa um evento, precisamos detectar
    if (is_victory(user, mapa)) {
      victory(tela, user);
      break;
    } else if (is_bonus(user, mapa)) {
      update_score(user, 1);
      // TODO: This is probably gonna require a flag
      // mvwaddstr(tela, 20, 20, "Você acabou de passar por um buraco de
      // minhoca!");
    } else if (is_obstaculo(user, mapa)) {
      user->vidas -= 1;

      if (user->vidas <= 0) {
        terminal_noraw();
        printf(VERMELHO
               "\n\nVocê se moveu em direção a um buraco negro e virou "
               "espaguete interstelar\n\n" RESET);
        break;
      }
    }

    if (is_limitedomapa(user, mapa)) {
      terminal_noraw();
      printf(
          VERMELHO
          "\n\nVocê ultrapassou o limite do UNIVERSO!!! e morreu.\n\n" RESET);
      break;
    }

    // Muda a posição do usuário no mapa efetivamente
    mapa[user->y][user->x] = 1;
    atualiza_tela(tela, user, mapa);
    sleep(1);

    atualizaMapa(mapa);
    if (is_obstaculo(user, mapa)) {
      user->vidas -= 1;

      if (user->vidas <= 0) {
        terminal_noraw();
        printf(
            VERMELHO
            "\n\nUm objeto estranho colidiu com sua nave e ela se desintegrou."
            "Não foi dessa vez, comandante.\n\n" RESET);
        break;
      }
    }
    atualiza_tela(tela, user, mapa);
  }

  terminal_noraw();
}

void popula_dados(struct UserData *user, int mapa[10][10]) {
  int y;
  for (y = 0; y < 10; ++y) {
    if (mapa[y][0] == 1) {
      user->x = 0;
      user->y = y;
    }
  }

  user->score = 0;
  user->vidas = 3;
}

/// detecta se colide com obstaculo.//
int is_obstaculo(struct UserData *user, int mapa[10][10]) {
  if (mapa[user->y][user->x] == 3) {
    return 1;
  }

  return 0;
}

int is_limitedomapa(struct UserData *user, int mapa[][10]) {
  if (user->y > 9 || user->x > 9 || user->y < 0 || user->x < 0) {
    return 1;
  }

  return 0;
}

void atualizaRankingDados(struct UserData *user, char nomes[3][30],
                          int scores[3]) {
  int counter, rankPos, resposta;

  // NOTE: Maybe it would be better to have the code for finding
  // repeated names as a function that signals wheter it found a repeated
  // name or not. Possibly even separating the detection of repeated names on
  // the ranking and the updating of the user's score on there, but it feels
  // like a great increase of cognitive overhead for very little benefit

  // checa se uma entrada com o nome do nosso usuário já existe no ranking.
  for (rankPos = 0; rankPos < 3; rankPos++) {
    if (!strcmp(user->nome, nomes[rankPos])) {
      printf(VERMELHO
             "\n\n\nJá existe uma entrada no ranking com o seu nome, %s. "
             "Substituindo Caso o número de pontos da partida atual tenha sido "
             "maior que o da anterior.\n\n" RESET,
             user->nome);

      if (user->score > scores[rankPos]) {
        strcpy(nomes[rankPos], user->nome);
        scores[rankPos] = user->score;
      }

      //  Caso o usuário já esteja no ranking, executar o código que segue
      //  não faz sentido
      return;
    }
  }

  // O que esse loop faz é essencialmente mover todas as posições a partir
  // da posição(inclusa) em que o usuário ficará no ranking para baixo
  for (counter = 0; counter < 3; counter++) {
    if (user->score > scores[counter]) {
      if (counter < 2) {
        // Já que cada entrada é sobrescrita pela posterior, só iteramos
        // até o penúltimo índice dos vetores.
        for (rankPos = counter; rankPos < 2; rankPos++) {
          strcpy(nomes[rankPos + 1], nomes[rankPos]);
          scores[rankPos + 1] = scores[rankPos];
        }
      }

      strcpy(nomes[counter], user->nome);
      scores[counter] = user->score;
      break;
    }
  }
}

void imprimeRanking(char nomes[3][30], int pontos[3]) {
  int i;
  printf("\n\n\nRanking atual:");
  for (i = 0; i < 3; i++) {
    printf("\n\t[%d] - %s - Crunopios: %d", i + 1, nomes[i], pontos[i]);
  }
}

void menuPrincipal() {
  setlocale(LC_ALL, "pt_BR.utf8");
  int mapa[10][10], at, resposta, scores[3];
  struct UserData user;
  char nome[30], nomes[3][30];
  const char *nomeArquivo = "ranking.txt";
  // TODA VEZ QUE VOCE DESEJAR GERAR UM NOVO MAPA, BASTA CHAMAR A FUNCAO
  // geraMapa, conforme o exemplo a seguir O primeiro par�metro � a sua matriz
  // 10x10, o segundo � o n�vel de dificuldade do jogo: 1-F�cil,
  // 2-Intermedi�rio, 3-Dif�cil
  // AQUI ESTOU USANDO O N�VEL DE DIFICULDADE 3 - DIF�CIL

  // popula ranking com valores nulos iniciais
  int rankingI;
  for (rankingI = 0; rankingI < 3; rankingI++) {
    scores[rankingI] = 0;
    strcpy(nomes[rankingI], "");
  }

  //  Melhor ler o ranking antes de iniciar o jogo, para evitar imprimir muito
  // lixo na tela no meio da partida
  printf("\n\nPrimeiro nós lemos o ranking armazenado no arquivo rank.dat: ");
  leRanking(nomes, scores);

  printf(
      VERDE
      "\n\n*Viagem nas estrelas, por Paulo Henrique, Matheus Melchiori e Luiz "
      "Filipe.\n\n\nOlá, comandante! Você é a responsável por guiar a "
      "nave Osíris até o planeta V.\nLá, conforme dito pelos maiores "
      "cientisdas da humanidade,você encontrará uma fonte de energia "
      "sustentável, o plasma, capaz de superar a crise energética que vem "
      "abalando a humanidade durante o nosso 3 milênio da era moderna. "
      "Confiamos em você!\n\n" RESET);
  printf(AMARELO "- Para seguirmos, informe o seu nome: " RESET);
  scanf("%[^\n]", user.nome);
  confereNome(user.nome);

  do {
    printf(MAGENTA
           "\n\n %s, antes de iniciar a sua jornada, algumas instruções:" RESET,
           user.nome);
    printf(VERDE
           "\n-- Os caracteres B que aparecem na tela são os buracos de "
           "minhoca "
           "que o ajudarão na sua jornada para o planeta Osíris. Passar "
           "por buracos de minhoca faz o seu número de crunopios, a "
           "moeda em circulação no planeta V, aumentar. Os osirianos "
           "atribuem esse comportamente a um bug introduzido no universo "
           "durante o big bang, mas ninguém sabe exatamente o porquê.\n-- Já "
           "os caracteres O representam obstáculos "
           "variados que irão danificar a "
           "sua nave, como cometas, meteoros e, quem sabe, ET's!\n-- O "
           "caractere V representa o planeta V. Se você chegar nesse "
           "caractere, "
           "você concluiu a sua missão!\n-- OBSERVAÇÃO: Se você perder as "
           "três "
           "vidas de sua nave, você irá morrer! Cuidado! \n\n\n" RESET);
    printf(
        AMARELO
        "Qual a dificuldade desejada?\n1. Fácil\n2. Difícil\n\nResp: " RESET);
    setbuf(stdin, NULL);
    scanf("%i", &user.resposta);

    confereResposta(user.resposta);
    geraMapa(mapa, user.resposta);

    // TODA VEZ QUE VOCE DESEJAR ATUALIZAR UM MAPA, BASTA CHAMAR A FUNCAO
    // atualizaMapa, conforme o exemplo a seguir

    game_loop(&user, mapa);
    atualizaRankingDados(&user, nomes, scores);
    atualizaRanking(nomes, scores);
    imprimeRanking(nomes, scores);

    printf(AMARELO
           "\n\nGostaria de jogar novamente?\n1.Sim\n2.Não\nResp: " RESET);

    setbuf(stdin, NULL);
    scanf("%i", &resposta);
    confereResposta(resposta);
    if (resposta == 2) {
      printf(AMARELO "\nAté depois!" RESET);
    }
  } while (resposta == 1);
}

int main() {
  menuPrincipal();
  return 0;
}

