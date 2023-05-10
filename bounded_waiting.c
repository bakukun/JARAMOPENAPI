/*
 * Copyright(c) 2021-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 * 2023-05-10 , 컴퓨터학부 , 2019004593 , 홍진수 , 색색의 문자열들이 각자 섞이지 않고 정해진 차례 안에 스레드가 lock을 얻어 순서대로 출력하도록 spin lock 기능 구현
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h> // 스핀락 구현을 위한 라이브러리
#include <stdatomic.h> // 스핀락 구현을 위한 라이브러리


#define N 8             /* 스레드 개수 */
#define RUNTIME 100000  /* 출력량을 제한하기 위한 실행시간 (마이크로초) */

/*
 * ANSI 컬러 코드: 출력을 쉽게 구분하기 위해서 사용한다.
 * 순서대로 BLK, RED, GRN, YEL, BLU, MAG, CYN, WHT, RESET을 의미한다.
 */
char *color[N+1] = {"\e[0;30m","\e[0;31m","\e[0;32m","\e[0;33m","\e[0;34m","\e[0;35m","\e[0;36m","\e[0;37m","\e[0m"};

/*
 * waiting[i]는 스레드 i가 임계구역에 들어가기 위해 기다리고 있음을 나타낸다.
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 * 원자적(atomic)으로 값을 변경하는 atomic_bool 타입 변수 lock을 사용하여 임계구역 진입 여부를 확인한다.
 * 추가로 이 lock의 값을 초기에 false로 설정한다.
 */
bool waiting[N];
bool alive = true;
atomic_bool lock = false;
/*
 * N 개의 스레드가 임계구역에 배타적으로 들어가기 위해 스핀락을 사용하여 동기화한다.
 */
void *worker(void *arg)
{
    int i = *(int *)arg; 
    int j;
    while (alive) {
        /*
         * 임계구역 진입 전, waiting[i]을 true로 설정하고, expected를 false로 초기화한다.
         * atomic_compare_exchange_weak 함수를 사용 => lock 변수의 값을 expected와 비교한다.
         * 이 때, lock이 false이고, expected와 일치(false)할 시 lock을 true로 변경한다.
         * 만약 lock이 true라 (다른 스레드가 스핀락을 보유중 이라서) expected와 불일치하고,
         * 이 조건들과 같이 waiting[i]이 true일시, 즉 기다려야 할 때 재시도를 위해 expected의 값을 다시 false로 설정한다.
         */
        waiting[i] = true;
        bool expected = false;

        while (!atomic_compare_exchange_weak(&lock, &expected, true) && waiting[i]){
            expected = false;
        }
        /*
         * 임계구역: 알파벳 문자를 한 줄에 40개씩 10줄 출력한다.
         */
        for (int k = 0; k < 400; ++k) {
            printf("%s%c%s", color[i], 'A'+i, color[N]);
            if ((k+1) % 40 == 0)
                printf("\n");
        }
        /*
         * 임계구역이 성공적으로 종료되고나서 스핀락을 반납한다.
         * 이 과정에서 waiting 배열 및 변수 j를 통해 다음 순서의 스레드가 스핀락을 얻을 수 있도록 한다.
         * j가 i와 같지 않고 다음 스레드가 스핀락을 기다리지 않는 경우에는 인덱스를 증가한다.
         * 만약 j와 i가 같아지면 기다리는 프로세스가 없다는 뜻이므로 lock을 풀지만,
         * 그 경우가 아니라면 Critical section에 들어가기를 기다리는 프로세스가 있기 때문에 lock을 건 상태에서 들어가도록 한다. 
         * 즉 해당 스레드에게 lock을 물려주는 효과가 있다.
         */
        j = (i+1) % N;
        while ((j != i ) && !waiting[j])
            j = (j+1) % N;
        if (j == i)
            lock = false;
        else
            waiting[j] = false;
        
    }
    
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[N];
    int i, id[N];

    /*
     * N 개의 자식 스레드를 생성한다.
     */
    for (i = 0; i < N; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, worker, id+i);
    }
    /*
     * 스레드가 출력하는 동안 RUNTIME 마이크로초 쉰다.
     * 이 시간으로 스레드의 출력량을 조절한다.
     */
    usleep(RUNTIME);
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
     * 메인함수를 종료한다.
     */
    return 0;
}
