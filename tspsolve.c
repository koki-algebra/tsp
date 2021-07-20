#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tspsolve.h"

#define COST_HISTORY_SIZE 256

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
      if (j == 128) {
        printf("\n");
      } else {
        printf(" ");
      }
    }
  }
}

// pathの出力
void print_path(int n, int* path) {
  printf("[ ");
  for (int i = 0; i < n; i++) {
    printf("%d ", path[i]);
  }
  printf("]\n");
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
      rnd = (int)floor(genrand() * n);  // 乱数生成
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
int calc_cost(int n, int* path, int cost_matrix[n][n]) {
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

// ランダム探索を実行する関数
void random_search(int n, int coordinate[n][2], int cost_matrix[n][n]) {
  // 試行回数入力
  int m;
  printf("ランダム探索試行回数: ");
  scanf("%d", &m);

  int cost[m];  // コスト
  int* path = NULL;
  int paths[m][n];
  char filename[128];  // pathを書き込むファイル名
  FILE *fp;  // ファイルポインタ
  int interval = 300;
  int ans;  // 暫定解のコスト
  int ans_index = 0;   // 暫定解のindex
  int ans_indexes[m];  // 暫定解のindexの推移を格納する配列

  // 乱数シード初期化
  sgenrand((unsigned)time(NULL));
  for (int i = 0; i < m; i++) {
    // パス生成
    path = gen_random_path(n);
    for (int j = 0; j < n; j++) {
      paths[i][j] = path[j];
    }
    // コスト計算
    cost[i] = calc_cost(n, path, cost_matrix);
    free(path);
  }

  // 暫定解が変化したタイミングi(0 <= i <= m)を配列に保存
  ans = cost[0];  // 暫定解初期化
  for (int i = 0; i < m; i++) {
    ans_indexes[i] = ans_index;
    if (ans > cost[i]) {
      ans = cost[i];
      ans_index = i;
    }
  }

  // ファイルへの書き込み(cost)
  sprintf(filename, "./cost/random/random-cost-%d.dat", m);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("error: Failed to open file!\n");
    exit(1);
  }
  for (int i = 0; i < m; i++) {
    if (i == 0 || (i + 1) % 10 == 0) {
      fprintf(fp, "%d %d\n", i + 1, cost[ans_indexes[i]]);
    }
  }
  fclose(fp);

  // ファイルへの書き込み(path)
  for (int i = 0; i < m; i++) {
    // 初回以降は100の倍数回目に書き込む
    if (i == 0 || (i + 1) % interval == 0) {
      // ファイルへの書き込み
      sprintf(filename, "./path/random/random-%d.dat", i+1);
      fp = fopen(filename, "w");
      if (fp == NULL) {
        printf("error: Failed to open file!\n");
        exit(1);
      }
      for (int j = 0; j <= n; j++) {
        if (j != n) {
          int x = coordinate[paths[ans_indexes[i]][j]][0];
          int y = coordinate[paths[ans_indexes[i]][j]][1];
          fprintf(fp, "%d %d\n", x, y);
        } else {  // 最後に開始都市を書き込む
          int x0 = coordinate[paths[ans_indexes[i]][0]][0];
          int y0 = coordinate[paths[ans_indexes[i]][0]][1];
          fprintf(fp, "%d %d\n", x0, y0);
        }
      }
      fclose(fp);
    }
  }
}

/*  山登り法  */

// pathのファイルへの書き込みを行う関数(m回目, 次元n)
void hc_path_fprintf(int m, int n, int* path, int coordinate[n][2]) {
  char filename[128];
  FILE *fp;

  // pathの書き込み
  sprintf(filename, "./path/hill-climbing/hill-climbing-%d.dat", m);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("error: Failed to open file!\n");
    exit(1);
  }
  for (int i = 0; i <= n; i++) {
    if (i != n) {
      fprintf(fp, "%d %d\n", coordinate[path[i]][0], coordinate[path[i]][1]);
    } else {
      fprintf(fp, "%d %d\n", coordinate[path[0]][0], coordinate[path[0]][1]);
    }
  }
  fclose(fp);
}

// 山登り法においてコストをファイルへ書き込む関数
void hc_cost_fprintf(int* costs) {
  char filename[128];
  FILE *fp;

  // costの書き込み
  sprintf(filename, "./cost/hill-climbing/hill-climbing-cost.dat");
  fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("error: Failed to open file!\n");
    exit(1);
  }
  for (int i = 0; i < COST_HISTORY_SIZE; i++) {
    if (costs[i] != 0) {
      fprintf(fp, "%d %d\n", i, costs[i]);
    }
  }
  fclose(fp);
}

// path[i]とpath[j]を入れ替えたnew_pathを返す関数
int* gen_neighborhood(int* path, int n, int i, int j) {
  // メモリ確保
  int* new_path = (int*)malloc(sizeof(int) * n);
  if (path == NULL) {
    printf("error: Failed to create the array.\n");
    exit(1);
  }

  // 配列のコピー
  for (int k = 0; k < n; k++) {
    new_path[k] = path[k];
  }

  // i番目とj番目を入れ替え
  new_path[i] = path[j];
  new_path[j] = path[i];

  return new_path;
}

// path[i]とpath[j]を両端として繋ぎ替えたnew_pathを返す関数
int* gen_2opt_nb(int* path, int n, int i, int j) {
  // i <= j でなければ交換
  if (i > j) {
    int tmp = i;
    i = j;
    j = tmp;
  }
  // メモリ確保
  int* new_path = (int*)malloc(sizeof(int) * n);
  if (path == NULL) {
    printf("error: Failed to create the array.\n");
    exit(1);
  }

  // 配列のコピー
  for (int k = 0; k < n; k++) {
    new_path[k] = path[k];
  }

  // 新しいpathを生成
  for (int k = i; k <= j; k++) {
    new_path[k] = path[j - k + i];
  }

  return new_path;
}

// 山登り法を実行する関数
void hill_climbing(int n, int cost_matrix[n][n], int coordinate[n][2]) {
  int nb_type;  // 0: ランダム交換, 1: 2-opt近傍
  int algo_type;  // 0: 通常, 1: 変種
  int end_flag = 0;  // 終了フラグ
  int find_flag = 0;  // 発見フラグ
  int count = 0;  // whileループの回数
  int* path = NULL;  // 暫定解
  int* nb_path = NULL;  // 近傍解
  int cost;  // 暫定解のコスト
  int cost_history[COST_HISTORY_SIZE] = {0};  // 暫定解のコストの履歴
  int nb_cost;  // 近傍解のコスト
  int ans_index[2];

  // ランダム交換 or 2opt-近傍
  printf("input '0' or '1'\n");
  printf("0: random exchange, 1: 2-opt neighbourhood\n");
  scanf("%d", &nb_type);

  // 通常 or 変種
  printf("input '0' or '1'\n");
  printf("0: normal, 1: variant\n");
  scanf("%d", &algo_type);

  // 初期解を生成する
  sgenrand((unsigned)time(NULL));
  path = gen_random_path(n);
  cost = calc_cost(n, path, cost_matrix);
  cost_history[0] = cost;

  if (nb_type == 0) {  // ランダム交換
    if (algo_type == 0) {  // 通常
      while (1) {
        find_flag = 0;
        count++;
        // 暫定解をファイルへ書き込む
        hc_path_fprintf(count, n, path, coordinate);
        // 現在の解の近傍の中から，最も良い解を選び近傍解とする
        for (int i = 0; i < n - 1; i++) {
          for (int j = i + 1; j < n; j++) {
            nb_path = gen_neighborhood(path, n, i, j);
            nb_cost = calc_cost(n, nb_path, cost_matrix);
            if (cost > nb_cost) {
              // コスト更新
              cost = nb_cost;
              ans_index[0] = i;  // path[i]と
              ans_index[1] = j;  // path[j]を入れ替えた時が暫定解
              find_flag = 1;  // 発見フラグを立てる
            }
          }
          // 入れ替えが起こらなければ終了フラグを立てる
          if (i == n - 2 && find_flag == 0) {
            end_flag = 1;
          }
        }
        // 終了フラグが立っていればループを抜けて終了
        if (end_flag) {
          break;
        }
        // 暫定解を更新
        path = gen_neighborhood(path, n, ans_index[0], ans_index[1]);
        // 履歴へ追加
        cost_history[count] = cost;
      }
    } else {  // 変種
      // 現在の解の近傍の中から，現在の解よりも良い解が見つかれば其れを近傍解とし，現在の解と近傍解を入れ替える
      while (1) {
        find_flag = 0;
        count++;
        // 暫定解をファイルへ書き込む
        hc_path_fprintf(count, n, path, coordinate);
        // 近傍解を探索する
        for (int i = 0; i < n - 1; i++) {
          if (find_flag) {
            break;
          }
          for (int j = i + 1; j < n - 1; j++) {
            nb_path = gen_neighborhood(path, n, i, j);
            nb_cost = calc_cost(n, nb_path, cost_matrix);
            // より良い解が見つかったら入れ替える
            if (cost > nb_cost) {
              // コストを更新
              cost = nb_cost;
              // 暫定解を更新
              int tmp = path[i];
              path[i] = path[j];
              path[j] = tmp;
              find_flag = 1;  // 発見フラグを立てる
              break;
            }
          }
          if (i == n - 2 && find_flag == 0) {  // 最後まで入れ替えが起こらなかったら終了フラグを立てる
            end_flag = 1;
          }
        }
        // 入れ替えが起こらなかった場合ループを抜けて終了
        if (end_flag) {
          break;
        }
        // 履歴へ追加
        cost_history[count] = cost;
      }
    }
  } else {  // 2-opt近傍
    if (algo_type == 0) {  // 通常
      while (1) {
        find_flag = 0;
        count++;
        // 暫定解をファイルへ書き込む
        hc_path_fprintf(count, n, path, coordinate);
        // 現在の解の近傍の中から，最も良い解を選び近傍解とする
        for (int i = 0; i < n - 1; i++) {
          for (int j = i + 1; j < n; j++) {
            nb_path = gen_2opt_nb(path, n, i, j);
            nb_cost = calc_cost(n, nb_path, cost_matrix);
            if (cost > nb_cost) {
              // コスト更新
              cost = nb_cost;
              ans_index[0] = i;
              ans_index[1] = j;
              find_flag = 1;  // 発見フラグを立てる
            }
          }
          // 入れ替えが起こらなければ終了フラグを立てる
          if (i == n - 2 && find_flag == 0) {
            end_flag = 1;
          }
        }
        // 終了フラグが立っていればループを抜けて終了
        if (end_flag) {
          break;
        }
        // 暫定解を更新
        path = gen_2opt_nb(path, n, ans_index[0], ans_index[1]);
        // 履歴へ追加
        cost_history[count] = cost;
      }
    } else {  // 変種
      while (1) {
        find_flag = 0;
        count++;
        // 暫定解をファイルへ書き込む
        hc_path_fprintf(count, n, path, coordinate);
        // 近傍を探索する
        for (int i = 0; i < n - 1; i++) {
          if (find_flag) {
            break;
          }
          for (int j = i + 1; j < n; j++) {
            nb_path = gen_2opt_nb(path, n, i, j);
            nb_cost = calc_cost(n, nb_path, cost_matrix);
            // より良い解が見つかったら入れ替える
            if (cost > nb_cost) {
              // コストを更新
              cost = nb_cost;
              // 暫定解を更新
              path = gen_2opt_nb(path, n, i, j);
              find_flag = 1;  // 発見フラグを立てる
              break;
            }
          }
          if (i == n - 2 && find_flag == 0) {  // 最後まで入れ替えが起こらなかったら終了フラグを立てる
            end_flag = 1;
          }
        }
        // 入れ替えが起こらなかった場合ループを抜けて終了
        if (end_flag) {
          break;
        }
        // 履歴へ追加
        cost_history[count] = cost;
      }
    }
  }

  // コストの履歴をファイルへ書き込む
  hc_cost_fprintf(cost_history);
  // メモリ開放
  free(path);
}


/*  シミュレーテッド・アニーリング法  */

// 温度減少関数
double calc_tmp(double a, double T) {
  // 温度減少率a (0 < a < 1), 温度T
  return (1 - a) * T;
}

// 反復回数を変化させる関数(T0: 初期温度, T: 現在の温度, R0: 初期反復回数)
int calc_length(double T0, double T, int R0) {
  double min = 0.5;
  double max = 1.5;
  // b(0) = b(T0) = min, b(T0/2) = max となるように係数a, cを設定する
  double a = 4 * (min - max) / (T0 * T0);
  double c = max;
  // b(T) = a(T - T0/2)^2 + c
  double b = a * (T - (T0 / 2.0)) * (T - (T0 / 2.0)) + c;

  return floor(b * R0);
}

// シミュレーテッド・アニーリング法を実行する関数
void simulated_annealing(int n, int cost_matrix[n][n]) {
  double T0;  // 初期温度
  double T;  // 温度
  int R0;  // 初期反復回数
  int R;  // 反復回数
  int D;  // 現在の解と近傍解のコストの差
  double a;  // 温度減少率
  double b;  // 温度Tに応じて反復回数を変化させる倍率b(T)
  int* path = NULL;  // 暫定解
  int* nb_path = NULL;  // 近傍解
  int nb_type;  // 近傍のタイプ (0: ランダム交換, 1: 2-opt近傍)
  int cost;  // 暫定解のコスト
  int nb_cost;  // 近傍解のコスト
  int end_flag = 0;

  // 初期温度と初期反復回数を設定する
  printf("input initial temperature\n");
  scanf("%lf", &T0);
  T = T0;
  printf("input initial iteration count\n");
  scanf("%d", &R0);
  R = R0;

  // 温度減少率と反復回数減少率を設定する
  printf("input temperature reduction rate a (0 < a < 1)\n");
  scanf("%lf", &a);

  // 近傍の種類を選択する
  printf("select neighborhood type '0' or '1'\n");
  printf("0: random exchange, 1: 2-opt neighborhood\n");
  scanf("%d", &nb_type);

  // 初期解を生成する
  sgenrand((unsigned)time(NULL));
  path = gen_random_path(n);

  // 終了条件を満たすまでループ
  while (1) {
    // R回探索
    int count = 0;  // R回のうち, 交換が起きなかった数
    for (int i = 0; i < R; i++) {
      // R0回のうち8割交換が起きなかったら終了フラグを立てる
      if (count > (R0 * 0.8)) {
        printf("There was almost no exchange!\n");
        end_flag = 1;
        break;
      }

      // path[k]とpath[l]を使って近傍解を作る
      int k, l;
      k = (int)floor(genrand() * n);
      // k != l にするためのバリデーション
      while (1) {
        l = (int)floor(genrand() * n);
        if (k != l) {
          break;
        }
      }
      if (nb_type == 0) {  // ランダム交換
        nb_path = gen_neighborhood(path, n, k, l);
      } else {  // 2-opt近傍
        nb_path = gen_2opt_nb(path, n, k, l);
      }

      // 暫定解と近傍解のコストを計算
      cost = calc_cost(n, path, cost_matrix);
      nb_cost = calc_cost(n, nb_path, cost_matrix);

      // コストの差を計算
      D = nb_cost - cost;

      // Dを元に解を更新する(しない場合はcountを1増やす)
      if (D <= 0) {
        // 暫定解の更新
        if (nb_type == 0) {  // ランダム交換
          path = gen_neighborhood(path, n, k, l);
        } else {  // 2-opt近傍
          path = gen_2opt_nb(path, n, k, l);
        }
      } else {
        // 確率exp(-D/T)で暫定解を更新する
        if (exp( (double)(-D) / T ) >= genrand()) {
          if (nb_type == 0) {  // ランダム交換
            path = gen_neighborhood(path, n, k, l);
          } else {  // 2-opt近傍
            path = gen_2opt_nb(path, n, k, l);
          }
        } else {
          count++;
        }
      }
    }

    // 温度更新
    T = calc_tmp(a, T);
    // 反復回数更新
    R = calc_length(T0, T, R0);

    // 温度が0に近ければ終了フラグを立てる
    if (T < 0.01) {
      printf("The temperature has dropped completely!\n");
      end_flag = 1;
    }

    // 終了フラグが立っていたらループを抜ける
    if (end_flag) {
      break;
    }
  }

  // 結果
  printf("result: cost = %d\n", cost);

  free(path);
}

int main(int argc, char *argv[]) {
  int n;          // 都市数
  int m;          // 試行回数
  FILE *fp;       // ファイルポインター
  char temp[100];
  int type = 1;

  // 引数の過不足に対するエラー処理
  if (argc != 3) {
    printf("Usage: sample <input_filename>\n");
    exit(1);
  }

  // 引数で指定されたファイル名を使用したファイルの読み込み
  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("file open error!\n");
    exit(1);
  }

  // 第3引数で探索法を決める
  if (strcmp(argv[2], "r") == 0) {
    printf("random search start!\n");
    type = 0;
  } else if (strcmp(argv[2], "hc") == 0) {
    printf("hill climbing start!\n");
    type = 1;
  } else if (strcmp(argv[2], "sa") == 0) {
    printf("simulated annealing start!\n");
    type = 2;
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

  if (type == 0) {
    // ランダム探索
    random_search(n, coordinate, cost_matrix);
  } else if (type == 1) {
    // 山登り法
    hill_climbing(n, cost_matrix, coordinate);
    // シミュレーテッド・アニーリング法
  } else if (type == 2) {
    simulated_annealing(n, cost_matrix);
  } else {
    printf("error: invalid argument!\n");
  }

  // 最後にファイルを閉じる
  fclose(fp);
  return 0;
}