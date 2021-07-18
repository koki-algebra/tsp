#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

int main(int argc, char *argv[]) {
  int n;          // 都市数
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

  print_cost(n, cost_matrix);

  int cost = simple_path(n, cost_matrix);
  printf("cost = %d\n", cost);

  // 最後にファイルを閉じる
  fclose(fp);
}