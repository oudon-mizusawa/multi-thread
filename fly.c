#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define WIDTH   78  /* スクリーン幅 */
#define HEIGHT  23  /* スクリーン高さ */
#define MAX_FLY 6   /* 描画するハエの数 */
const char *flyMarkList = "o@*+.#"; /* ハエの描画文字一覧 */
#define DRAW_CYCLE  50  /* 描画周期(ミリ秒) */
#define MIN_SPEED   1.0 /* ハエの最低移動速度 */
#define MAX_SPEED   20.0    /* ハエの最大移動速度 */

int stopRequest;    /* スレッド終了フラグ */

pthread_mutex_t mutex;

/*
* ミリ秒単位でスリープする
*/
void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/*
* minValue以上maxValue未満のランダム値を得る
*/
double randDouble(double minValue, double maxValue) {
    return minValue + (double)rand() / ((double)RAND_MAX+1) * (maxValue - minValue);
}

/*
* 画面クリア
*/
void clearScreen(){
    fputs("\033[2J", stdout);   // このエスケープコードをターミナルに送ると画面がクリアされる
}

/*
* カーソル移動
*/
void moveCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);    /* このエスケープコードをターミナルに送るとカーソル位置がx, yになる */
}

/*
* ハエ構造体
*/
typedef struct {
    char mark;  /* 表示キャラクタ */
    double x, y;    /* 座標 */
    double angle;   /* 移動方向(角度) */
    double speed;   /* 移動速度(ピクセル/秒) */

} Fly;

Fly flyList[MAX_FLY];

// initialize state of fly randomly
void FlyInitRandom(Fly *fly, char mark_)
{
    fly->mark = mark_;
    fly->x = randDouble(0, (double)(WIDTH - 1));
    fly->y = randDouble(0, (double)(HEIGHT - 1));
    fly->angle = randDouble(0, (M_2_PI));
    fly->speed = randDouble(MIN_SPEED, MAX_SPEED);
}

// move fly
void FlyMove(Fly *fly) {
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);

    // change vector at collision along with edge of X direction
    if (fly->x < 0) {
        fly->x = 0;
        fly->angle = M_PI - fly->angle;
    } else if (fly->x > WIDTH - 1) {
        fly->x = WIDTH - 1;
        fly->angle = M_PI - fly->angle;
    }

    // change vector at collision along with edge of Y direction
    if (fly->y < 0) {
        fly->y = 0;
        fly->angle = - fly->angle;
    } else if (fly->y > HEIGHT - 1) {
        fly->y = HEIGHT - 1;
        fly->angle = -fly->angle;
    }
}

/*
* ハエが指定座標にあるか
*/
int FlyIsAt(const Fly *fly, int x, int y) {
    return ((int)(fly->x) == x) && ((int)(fly->y) == y);
}


/*
* ハエを動かし続けるスレッド
*/
void* doMove(void* arg) {
    Fly* fly = (Fly*)arg;

    while (!stopRequest)
    {
        pthread_mutex_lock(&mutex);
        FlyMove(fly);
        pthread_mutex_unlock(&mutex);
        mSleep((int)(1000.0 / fly->speed));
    }
    return NULL;
}

// draw screen
void drawScreen() {
    int x, y;
    char ch;
    int i;

    moveCursor(0, 0);

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            ch = 0;
            // display mark if fly existing at x, y coordinates.
            for (i = 0; i < MAX_FLY; i++) {
                if (FlyIsAt(&flyList[i], x, y)) {
                    ch = flyList[i].mark;
                    break;
                }
            }

            if (ch != 0) {
                putchar(ch);
            } else if ((y == 0) || (y == HEIGHT - 1)) {
                putchar('-');
            } else if ((x == 0) || (x == WIDTH - 1)) {
                putchar('|');
            } else {
                putchar(' ');
            }
        }
        putchar('\n');
    }
}

/* 
* スクリーンを描画し続けるスレッド
*/
void* doDraw(void* arg) {
    while(!stopRequest) {
        pthread_mutex_lock(&mutex);
        drawScreen();
        pthread_mutex_unlock(&mutex);
        mSleep(DRAW_CYCLE);
    }
    return NULL;
}

// How to compile is below here. added warning ignore option.
// "gcc fly.c -o fly -lpthread -lm -w"
int main()
{
    pthread_t drawThread;
    pthread_t moveThread[MAX_FLY];
    int i;
    char buf[40];

    // initialize
    srand((unsigned int)time(NULL));
    pthread_mutex_init(&mutex, NULL);
    clearScreen();

    for  (i = 0; i < MAX_FLY; i++)
    {
        FlyInitRandom(&flyList[i], flyMarkList[i]);
    }

    for (i = 0; i < MAX_FLY; i++)
    {
        pthread_create(&moveThread[i], NULL, doMove, (void *)&flyList[i]);
    }

    pthread_create(&drawThread, NULL, doDraw, NULL);

    fgets(buf, sizeof(buf), stdin);
    stopRequest = 1;
    pthread_join(moveThread[i], NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
