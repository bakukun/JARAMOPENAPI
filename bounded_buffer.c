/*
 * Copyright(c) 2021-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 * 2023-05-10 , 컴퓨터학부 , 2019004593 , 홍진수 , 생산자와 소비자가 문제 없이 아이템을 버퍼에서 주고 받을수 있도록 동기화를 하기 위해 spin lock 기능 구현
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h> // 스핀락 구현을 위한 라이브러리
#include <stdatomic.h> // 스핀락 구현을 위한 라이브러리

#define N 8
#define MAX 1024
#define BUFSIZE 4
#define RED "\e[0;31m"
#define RESET "\e[0m"
/*
 * 생산자와 소비자가 공유할 버퍼를 만들고 필요한 변수를 초기화한다.
 */
int buffer[BUFSIZE];
int in = 0;
int out = 0;
int counter = 0;
int next_item = 0;
/*
 * 생산된 아이템과 소비된 아이템의 로그와 개수를 기록하기 위한 변수
 */
int task_log[MAX][2];
int produced = 0;
int consumed = 0;
/*
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 * 원자적(atomic)으로 값을 변경하는 atomic_bool 타입 변수 lock을 사용하여 임계구역 진입 여부를 확인한다.
 * 추가로 이 lock의 값을 초기에 false로 설정한다.
 */
bool alive = true;
atomic_bool lock = false;

/*
 * 생산자 스레드로 실행할 함수이다. 아이템을 생성하여 버퍼에 넣는다.
 */
void *producer(void *arg)
{
    int i = *(int *)arg;
    int item;
    
    while (alive) {
        /*
         * 새로운 아이템을 생산하여 버퍼에 넣고 관련 변수를 갱신한다.
         * 임계구역 진입 전, expected를 false로 초기화한다.
         * atomic_compare_exchange_weak 함수를 사용 => lock 변수의 값을 expected와 비교한다.
         * 이 때, lock이 false이고, expected와 일치(false)할 시 lock을 true로 변경한다.
         * 만약 lock이 true라 (다른 스레드가 스핀락을 보유중 이라서) expected와 불일치하고,
         * 이 조건들과 같이 waiting[i]이 true일시, 즉 기다려야 할 때 재시도를 위해 expected의 값을 다시 false로 설정한다.
         */
        bool expected = false;
        while (!atomic_compare_exchange_weak(&lock, &expected, true)) {
            expected = false;
        }
        /*
         * 버퍼가 가득 찬 상태인지 counter 변수를 통해 확인한다.
         * 만약 버퍼가 가득찬 상태라면 lock 변수를 false로 설정하여 다른 스레드가 접근할 수 있도록 해제한다.
         * 또한 버퍼가 가득 찬 상태이므로 생산자 스레드는 현재 반복을 건너뛰고 다음 반복으로 진행한다.
         */
        if (counter == BUFSIZE) {
            lock = false;
            continue;
        }

        item = next_item++;
        buffer[in] = item;
        in = (in + 1) % BUFSIZE;
        counter++;

        /*
         * 생산자를 기록하고 중복생산이 아닌지 검증한다.
         * 
         */
        if (task_log[item][0] == -1) {
            task_log[item][0] = i;
            produced++;
            lock = false; // 임계 영역을 벗어날 때 lock을 false로 변경하여 다른 스레드가 접근할 수 있도록 해제한다.
        }
        else {
            lock = false; // 임계 영역을 벗어날 때 lock을 false로 변경하여 다른 스레드가 접근할 수 있도록 해제한다.
            printf("<P%d,%d>....ERROR: 아이템 %d 중복생산\n", i, item, item);
            continue;
        }
        /*
         * 생산한 아이템을 출력한다.
         */
        printf("<P%d,%d>\n", i, item);
        

    }
    pthread_exit(NULL);
}

/*
 * 소비자 스레드로 실행할 함수이다. 버퍼에서 아이템을 읽고 출력한다.
 */
void *consumer(void *arg)
{
    int i = *(int *)arg;
    int item;
    
    while (alive) {
        /*
         * 버퍼에서 아이템을 꺼내고 관련 변수를 갱신한다.
         * 임계구역 진입 전, expected를 false로 초기화한다.
         * atomic_compare_exchange_weak 함수를 사용 => lock 변수의 값을 expected와 비교한다.
         * 이 때, lock이 false이고, expected와 일치(false)할 시 lock을 true로 변경한다.
         * 만약 lock이 true라 (다른 스레드가 스핀락을 보유중 이라서) expected와 불일치하고,
         * 이 조건들과 같이 waiting[i]이 true일시, 즉 기다려야 할 때 재시도를 위해 expected의 값을 다시 false로 설정한다.
         */
        bool expected = false;
        while (!atomic_compare_exchange_weak(&lock, &expected, true))
            expected = false;
        /*
         * 버퍼가 비어 있는 상태인지 counter 변수를 통해 확인한다.
         * 만약 버퍼가 빈 상태라면 lock 변수를 false로 설정하여 다른 스레드가 접근할 수 있도록 해제한다.
         * 또한 버퍼가 빈 상태이므로 소비자 스레드는 현재 반복을 건너뛰고 다음 반복으로 진행한다.
         */
        if (counter == 0) {
            lock = false;
            continue;
        }

        item = buffer[out];
        out = (out + 1) % BUFSIZE;
        counter--;
        /*
         * 소비자를 기록하고 미생산 또는 중복소비 아닌지 검증한다.
         */

        if (task_log[item][0] == -1) {        
            lock = false; // 임계 영역을 벗어날 때 lock을 false로 변경하여 다른 스레드가 접근할 수 있도록 해제한다.
            printf(RED"<C%d,%d>"RESET"....ERROR: 아이템 %d 미생산\n", i, item, item);
            continue;
        }
        else if (task_log[item][1] == -1) {
            task_log[item][1] = i;
            consumed++;
            lock = false; // 임계 영역을 벗어날 때 lock을 false로 변경하여 다른 스레드가 접근할 수 있도록 해제한다.

        }
        else {
            lock = false; // 임계 영역을 벗어날 때 lock을 false로 변경하여 다른 스레드가 접근할 수 있도록 해제한다.
            printf(RED"<C%d,%d>"RESET"....ERROR: 아이템 %d 중복소비\n", i, item, item);
            continue;
        }
        /*
         * 소비할 아이템을 빨간색으로 출력한다.
         */
        
        printf(RED"<C%d,%d>"RESET"\n", i, item);
        
    }
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[N];
    int i, id[N];

    /*
     * 생산자와 소비자를 기록하기 위한 logs 배열을 초기화한다.
     */
    for (i = 0; i < MAX; ++i)
        task_log[i][0] = task_log[i][1] = -1;
    /*
     * N/2 개의 소비자 스레드를 생성한다.
     */
    for (i = 0; i < N/2; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, consumer, id+i);
    }
    /*
     * N/2 개의 생산자 스레드를 생성한다.
     */
    for (i = N/2; i < N; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, producer, id+i);
    }
    /*
     * 스레드가 출력하는 동안 1 밀리초 쉰다.
     * 이 시간으로 스레드의 출력량을 조절한다.
     */
    usleep(10000);
    /*
     * 스레드가 자연스럽게 무한 루프를 빠져나올 수 있게 한다.
     */
    alive = false;
    /*
     * 자식 스레드가 종료될 때까지 기다린다.
     */
    for (i = 0; i < N; ++i)
        pthread_join(tid[i], NULL);
    /*
     * 생산된 아이템을 건너뛰지 않고 소비했는지 검증한다.
     */
    for (i = 0; i < consumed; ++i)
        if (task_log[i][1] == -1) {
            printf("....ERROR: 아이템 %d 미소비\n", i);
            return 1;
        }
    /*
     * 생산된 아이템의 개수와 소비된 아이템의 개수를 출력한다.
     */
    if (next_item == produced) {
        printf("Total %d items were produced.\n", produced);
        printf("Total %d items were consumed.\n", consumed);
    }
    else {
        printf("....ERROR: 생산량 불일치\n");
        return 1;
    }
    /*
     * 메인함수를 종료한다.
     */
    return 0;
}
