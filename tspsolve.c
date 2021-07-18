#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tspsolve.h"

// 座標(x_i, y_i)と(x_j, y_j)のユークリッド距離を返す関数
int d(int x_i, int y_i, int x_j, int y_j) {
  double distance = sqrt((x_i - x_j) * (x_i - x_j) + (y_i - y_j) * (y_i - y_j));
  return round(distance);
}

// コスト行列の出力
void print_cost(int n, int cost_matrix[n][n]) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d", cost_matrix[i][j]);
      if (j == 50) {
        printf("\n");
      } else {
        printf(" ");
      }
    }
  }
}

// 単純巡回路のコスト計算
int simple_path(int n, int cost_matrix[n][n]) {
  int cost = 0;
  // 開始都市から最後の都市までのコストを計算
  for (int i = 0; i < n - 1; i++) {
    cost += cost_matrix[i][i + 1];
  }
  // 最後の都市から開始都市へ戻ってくる際のコストを加算
  cost += cost_matrix[0][n - 1];
  return cost;
}

// ランダム探索
// ランダムの巡回路を返す関数
int* gen_random_path(int n) {
  int* path = NULL;
  path = (int*)malloc(sizeof(int) * n);
  if (path == NULL) {
    printf("error: Failed to create the array.\n");
    exit(1);
  }

  int rnd;
  int flag;

  // pathをランダム生成
  for (int i = 0; i < n; i++) {
    do {
      flag = 0;  // フラグ初期化
      rnd = (int)floor(genrand() * 51);  // 乱数生成
      // 乱数に重複がないか確認
      for (int j = 0; j < i; j++) {
        // 重複が見つかった場合
        if (path[j] == rnd) {
          // フラグを立てて乱数を再生成
          flag = 1;
          break;
        }
      }
    } while(flag);
    // 乱数を格納
    path[i] = rnd;
  }

  return path;
}

// パスからコストを計算する関数
int calc_ransearch_cost(int n, int* path, int cost_matrix[n][n]) {
  int cost = 0;  // コスト

  // コストを計算
  for (int i = 0; i < n; i++) {
    if (i == n - 1) {  // 最後の都市から開始の都市に
      cost += cost_matrix[path[i]][path[0]];
    } else {
      cost += cost_matrix[path[i]][path[i + 1]];
    }
  }

  return cost;
}

void random_search(int n, int coordinate[n][2], int cost_matrix[n][n]) {
  // 試行回数入力
  int m;
  printf("ランダム探索試行回数: ");
  scanf("%d", &m);

  int cost[m];  // コスト
  int* path = NULL;
  int paths[m][n];
  char filename[30];  // pathを書き込むファイル名
  FILE *fp;  // ファイルポインタ
  int ans;  // 暫定解のコスト
  int interval = 100;

  // 乱数シード初期化
  sgenrand((unsigned)time(NULL));
  for (int i = 0; i < m; i++) {
    // パス生成
    path = gen_random_path(n);
    for (int j = 0; j < n; j++) {
      paths[i][j] = path[j];
    }
    // コスト計算
    cost[i] = calc_ransearch_cost(n, path, cost_matrix);
    free(path);
  }

  // ファイルへの書き込み
  for (int i = 0; i < m; i++) {
    // 初回以降は100の倍数回目に書き込む
    if (i == 0 || (i + 1) % interval == 0 ) {
      // ファイルへの書き込み
      sprintf(filename, "random-%d.dat", i+1);
      fp = fopen(filename, "w");
      if (fp == NULL) {
        printf("error: Failed to open file!\n");
        exit(1);
      }
      for (int j = 0; j <= n; j++) {
        if (j != n) {
          int x = coordinate[paths[i][j]][0];
          int y = coordinate[paths[i][j]][1];
          fprintf(fp, "%d %d\n", x, y);
        } else {  // 最後に開始都市を書き込む
          int x0 = coordinate[paths[i][0]][0];
          int y0 = coordinate[paths[i][0]][1];
          fprintf(fp, "%d %d\n", x0, y0);
        }
      }
      fclose(fp);
    }
  }
}

int main(int argc, char *argv[]) {
  int n;          // 都市数
  int m;          // 試行回数
  FILE *fp;       // ファイルポインター
  char temp[100];

  // 引数の過不足に対するエラー処理
  if (argc != 2) {
    printf("Usage: sample <input_filename>\n");
    exit(1);
  }

  // 引数で指定されたファイル名を使用したファイルの読み込み
  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("file open error!\n");
    exit(1);
  }

  /* 次元(都市数)の取得 */
  do {
    fscanf(fp, "%s", temp);
    // DIMENSIONの項目を読み込むまで空読み
    if (strcmp("DIMENSION", temp) == 0) {
      fscanf(fp, "%s", temp);
      break;
    }
    // DIMENSIONの後に「:」が入っている場合
    if (strcmp("DIMENSION :", temp) == 0) {
      break;
    }
  } while (1);

  fscanf(fp, "%d", &n);  // 次元(都市数)の読み込み


  /* 都市間のコスト(ユークリッド距離)計算 */
  int coordinate[n][2];      // 都市の座標を格納する配列
  int cost_matrix[n][n];  // 都市間のコストを格納する行列

  do {
    fscanf(fp, "%s", temp);
    if (strcmp("NODE_COORD_SECTION", temp) == 0) {
      break;
    }
  } while (1);

  // 都市の座標を格納
  for (int i = 0; i < n; i++) {
    int x_i;
    int y_i;
    fscanf(fp, "%s", temp);
    fscanf(fp, "%d", &x_i);
    fscanf(fp, "%d", &y_i);
    coordinate[i][0] = x_i;
    coordinate[i][1] = y_i;
  }

  // ユークリッド距離を計算
  for (int i = 0; i < n; i++) {
    int x_i = coordinate[i][0];
    int y_i = coordinate[i][1];
    for (int j = 0; j < n; j++) {
      int x_j = coordinate[j][0];
      int y_j = coordinate[j][1];
      cost_matrix[i][j] = d(x_i, y_i, x_j, y_j);
    }
  }

  // ランダム探索
  random_search(n, coordinate, cost_matrix);

  // 最後にファイルを閉じる
  fclose(fp);
}