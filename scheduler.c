#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_QUANTUM 5 //RR에 대한 time quantum
#define ALGORITHM_NUM 6 //알고리즘 개수
int processNum = 0; //process 개수

typedef struct {
    int pid; //1~N 랜덤, unique
    int arrivalTime; //0~10 랜덤
    int CPUburst; //10~30 랜덤
    int IOburst; //0~5 랜덤, 0 초과 시 1만큼 CPUburst 후 IOburst 진행
    int priority; //1~N 랜덤, 값이 낮을수록 높은 우선순위
    int CPUremaining;
    int IOremaining;
    int waitingTime;
    int turnaroundTime;

} process;
process** processes = NULL; 

typedef struct {
	double avgWaiting;
	double avgTurnaround;
} evaluation;
evaluation* evals[ALGORITHM_NUM] = {NULL};

char *algorithms[ALGORITHM_NUM] = {"First Come First Served", "Non-Preemptive Shortest Job First", "Preemptive Shortest Job First", "Non-Preemptive Priority", "Preemptive Priority", "Round Robin"}; 
int algCount = 0; //현재 몇번째 알고리즘인지 카운트

void sort_arrival(process** processes, int num){

    //sort processes (arrival time 작은 순서로, arrival time 같을 시 pid 작은 순서로)
    for (int i = 0; i < num - 1; i++) {
        for (int j = 0; j < num - i - 1; j++) {
            if (processes[j]->arrivalTime > processes[j + 1]->arrivalTime ||
                (processes[j]->arrivalTime == processes[j + 1]->arrivalTime && 
                processes[j]->pid > processes[j + 1]->pid)) {
                
                process* temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

void sort_burst(process** processes, int num){

    //sort processes (CPU remaining burst time 작은 순서로, burst time 같을 시 arrival time이 작은 순서로, arrival time도 같으면 pid가 작은 순서로)
    for (int i = 0; i < num - 1; i++) {
        for (int j = 0; j < num - i - 1; j++) {
            if (processes[j]->CPUremaining > processes[j + 1]->CPUremaining ||
                (processes[j]->CPUremaining == processes[j + 1]->CPUremaining &&
                 (processes[j]->arrivalTime > processes[j + 1]->arrivalTime ||
                  (processes[j]->arrivalTime == processes[j + 1]->arrivalTime &&
                   processes[j]->pid > processes[j + 1]->pid)))) {
                
                process* temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

void sort_priority(process** processes, int num){

    //sort processes (priority 작은 순서로, priority 같을 시 arrival time이 작은 순서로, arrival time도 같으면 pid가 작은 순서로)
    for (int i = 0; i < num - 1; i++) {
        for (int j = 0; j < num - i - 1; j++) {
            if (processes[j]->priority > processes[j + 1]->priority || 
                (processes[j]->priority == processes[j + 1]->priority && 
                processes[j]->arrivalTime > processes[j + 1]->arrivalTime) || 
                (processes[j]->priority == processes[j + 1]->priority && 
                processes[j]->arrivalTime == processes[j + 1]->arrivalTime && 
                processes[j]->pid > processes[j + 1]->pid)) {
                
                process* temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

void CreateProcess(){
    printf("Number of processes: ");
    scanf("%d", &processNum); //process 개수 입력받기
    processes = malloc(processNum * sizeof(process*));
    if (processes == NULL) {
        printf("malloc error");
        return;}

    //process 데이터 생성
    srand(time(NULL));
    for (int i = 0; i < processNum; i++) {
        processes[i] = malloc(sizeof(process));
        if (processes[i] == NULL) {
        printf("malloc error");
        return;}

        processes[i]->pid = i+1; //1~N 랜덤, unique
        processes[i]->arrivalTime = rand() % 11; //0~10 랜덤
        processes[i]->CPUburst = (rand() % 21) + 10; //10~30 랜덤
        processes[i]->IOburst = rand() % 6; //0~5 랜덤, 0 초과 시 1만큼 CPUburst 후 IOburst 진행
        processes[i]->priority = (rand() % processNum) + 1; //1~N 랜덤, 값이 낮을수록 높은 우선순위
        processes[i]->CPUremaining = processes[i]->CPUburst;
        processes[i]->IOremaining = processes[i]->IOburst;
        processes[i]->waitingTime = 0;
        processes[i]->turnaroundTime = 0;
    }
    sort_arrival(processes, processNum); //arrival time으로 정렬

    //각 프로세스 정보 출력
    for (int i = 0; i < processNum; i++) {
        printf("[Pid: %d, Arrival Time: %d, CPU Burst: %d, I/O Burst: %d, Priority: %d]\n",
               processes[i]->pid, processes[i]->arrivalTime, processes[i]->CPUburst, processes[i]->IOburst, processes[i]->priority);
    }
    printf("\n");
    
    for (int i = 0; i < ALGORITHM_NUM; i++) {//evals 배열 초기화
        evals[i] = malloc(sizeof(evaluation));
        if (evals[i] == NULL) {
        printf("malloc error");
        return;}
        
    }
}

//한 알고리즘에 대한 각 프로세스 정보 출력 & evals 배열 update
void EvaluateOne(process** p){
    printf("\n");

    //각 프로세스 정보 출력
    for (int i = 0; i < processNum; i++) {
        printf("[Pid: %d, Waiting Time: %d, Turnaround Time: %d]\n",
               p[i]->pid, p[i]->waitingTime, p[i]->turnaroundTime);
    }
    printf("\n");

    //evals update
    double avgWaiting = 0;
	double avgTurnaround = 0;

    for(int i=0; i<processNum; i++){
        avgWaiting += (double)p[i]->waitingTime;
        avgTurnaround += (double)p[i]->turnaroundTime;
    }

    avgWaiting /= (double)processNum;
    avgTurnaround /= (double)processNum;

    evals[algCount]->avgWaiting = avgWaiting; //이후 EvaluateAll을 위해 저장해둠
    evals[algCount]->avgTurnaround = avgTurnaround;

    algCount += 1;

    printf("Average waiting time: %.2f\n", avgWaiting);
	printf("Average turnaround time: %.2f\n", avgTurnaround);
    printf("===============================================================\n");
}

//마지막으로 evals 배열의 각각 algorithm 별 결과 비교
void EvaluateAll(){ //결과 동일할 경우 순서 상 첫 알고리즘으로 출력

    //Minimum average waiting time
    int minIndex=0;
    for(int i=1; i<ALGORITHM_NUM; i++){
        if(evals[i]->avgWaiting < evals[minIndex]->avgWaiting){
            minIndex = i;
        }
    }
    printf("Minimum average waiting time: %s\n", algorithms[minIndex]);

    //Minimum average turnaround time
    minIndex=0;
    for(int i=1; i<ALGORITHM_NUM; i++){
        if(evals[i]->avgTurnaround < evals[minIndex]->avgTurnaround){
            minIndex = i;
        }
    }
    printf("Minimum average turnaround time: %s\n", algorithms[minIndex]);

}

process** copy_processes(){ //deep copy
    process** p = malloc(sizeof(process*) * processNum);
    if (p == NULL) {
        printf("malloc error");
        return NULL;}
    
    for (int i = 0; i < processNum; i++) {
        p[i] = malloc(sizeof(process));
        if (p[i] == NULL) {
        printf("malloc error");
        return NULL;}

        p[i]->pid = processes[i]->pid;
        p[i]->arrivalTime = processes[i]->arrivalTime;
        p[i]->CPUburst = processes[i]->CPUburst;
        p[i]->IOburst = processes[i]->IOburst;
        p[i]->priority = processes[i]->priority;
        p[i]->CPUremaining = processes[i]->CPUremaining;
        p[i]->IOremaining = processes[i]->IOremaining;
        p[i]->waitingTime = processes[i]->waitingTime;
        p[i]->turnaroundTime = processes[i]->turnaroundTime;
    }

    return p;
}

void FCFS(){
    printf("<First Come First Served Algorithm>\n");
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사, arrival time으로 정렬되어 있음

    //FCFS 알고리즘: arrival time 같을 시 pid 작은 순서로
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    process* runningProcess = NULL;

    while (finished < processNum) {

        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        sort_arrival(readyQueue, readyCount); //arrival time으로 정렬
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }
    
    EvaluateOne(p); //한 알고리즘에 대한 각 프로세스 정보 출력 & evals 배열 update
    
    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

void NonPreemptive_SJF(){
    printf("<Non-preemptive Shortest Job First Algorithm>\n");
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사

    //NP_SJF 알고리즘: CPUburst 같으면 FCFS
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    process* runningProcess = NULL;

    while (finished < processNum) {

        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        sort_burst(readyQueue, readyCount); //CPU burst time으로 정렬
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;
            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }

    EvaluateOne(p);

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

void Preemptive_SJF(){
    printf("<Preemptive Shortest Job First Algorithm>\n");
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사

    //P_SJF 알고리즘: CPUburst 같으면 FCFS
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    process* runningProcess = NULL;

    while (finished < processNum) {

        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        sort_burst(readyQueue, readyCount); //CPU burst time으로 정렬
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함
            if(readyCount > 0){ //레디큐 다시 확인해서 필요시 preemption
                if(runningProcess->CPUremaining > readyQueue[0]->CPUremaining){ //이미 수행 중일 경우 burst 같을 시 기존 프로세스 유지 (pid 비교 X)
                    readyQueue[readyCount++] = runningProcess; //기존 프로세스를 레디큐에 다시 넣기
                    runningProcess = readyQueue[0];
                    for (int j = 1; j < readyCount; j++) { //새 프로세스를 레디큐에서 제거
                        readyQueue[j - 1] = readyQueue[j];}
                    readyCount--;
                }
            }
            runningProcess->CPUremaining--; //CPU burst
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }

    EvaluateOne(p);

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

void NonPreemptive_Priority(){
    printf("<Non-preemptive Priority Algorithm>\n");
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사

    //N_P 알고리즘: priority 같으면 FCFS
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    process* runningProcess = NULL;

    while (finished < processNum) {

        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        sort_priority(readyQueue, readyCount); //priority로 정렬
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;
            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }


    EvaluateOne(p);

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

void Preemptive_Priority(){
    printf("<Preemptive Priority Algorithm>\n");
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사

    //P_P 알고리즘: priority 같으면 FCFS
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    process* runningProcess = NULL;
    while (finished < processNum) {

        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        sort_priority(readyQueue, readyCount); //priority로 정렬
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함
            if(readyCount > 0){ //레디큐 다시 확인해서 필요시 preemption
                if(runningProcess->priority > readyQueue[0]->priority){ //이미 수행 중일 경우 priority 같을 시 기존 프로세스 유지 (pid 비교 X)
                    readyQueue[readyCount++] = runningProcess; //기존 프로세스를 레디큐에 다시 넣기
                    runningProcess = readyQueue[0];
                    for (int j = 1; j < readyCount; j++) { //새 프로세스를 레디큐에서 제거
                        readyQueue[j - 1] = readyQueue[j];}
                    readyCount--;
                }
            }
            runningProcess->CPUremaining--; //CPU burst
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;
            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }


    EvaluateOne(p);

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

void RR(){
    printf("<Round Robin Algorithm (time quantum: %d)>\n",TIME_QUANTUM);
    //Config: readyQueue, waitingQueue, p 설정
    process** readyQueue = malloc(sizeof(process*) * processNum);
    if (readyQueue == NULL) {
        printf("malloc error");
        return;}
    process** waitingQueue = malloc(sizeof(process*) * processNum);
    if (waitingQueue == NULL) {
        printf("malloc error");
        return;}
    process** p = NULL; 
    p = copy_processes(); //p에 processes를 복사

    //RR 알고리즘 (FCFS with time quantum): preemption 시 현재 레디큐의 첫 프로세스 선택
    int readyCount = 0;
    int waitingCount = 0;
    int currentTime = 0;
    int finished = 0;
    int checked = 0;
    int quantum = 0;
    process* runningProcess = NULL;

    while (finished < processNum) {
        
        printf("Time %d: ", currentTime);
        
        while (checked < processNum && p[checked]->arrivalTime <= currentTime) { //도착한 프로세스들을 레디큐에 업데이트
            readyQueue[readyCount++] = p[checked++];
        }
        
        if (runningProcess != NULL) { //작업 중인 프로세스 존재함
            
            if(readyCount > 0){ 
                if(quantum >= TIME_QUANTUM){ //레디큐 다시 확인해서 필요시 preemption
                    readyQueue[readyCount++] = runningProcess; //기존 프로세스를 레디큐에 다시 넣기
                    runningProcess = readyQueue[0];
                    for (int j = 1; j < readyCount; j++) { //새 프로세스를 레디큐에서 제거
                        readyQueue[j - 1] = readyQueue[j];}
                    readyCount--;
                    quantum = 0;
                }
            }

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            quantum++;
            printf("pid %d [running], ", runningProcess->pid);

        } else if (readyCount > 0) { //레디큐에서 새로운 프로세스 가져오기

            runningProcess = readyQueue[0]; 
            for (int j = 1; j < readyCount; j++) { //레디큐에서 제거 후 CPU burst
                readyQueue[j - 1] = readyQueue[j];}
            readyCount--;
            quantum = 0;

            runningProcess->CPUremaining--;
            runningProcess->turnaroundTime++;
            quantum++;
            printf("pid %d [running], ", runningProcess->pid);

        } else { //idle
            printf("[CPU idle], ");
        }
        if(runningProcess != NULL){
            if (runningProcess->CPUremaining <= 0) { //terminated
                printf("pid %d [terminated], ", runningProcess->pid);
                runningProcess = NULL;
                finished++;
            }
        }
        //레디큐 프로세스들 기다림
        for(int i = 0; i < readyCount; i++) { 
            readyQueue[i]->waitingTime++;
            readyQueue[i]->turnaroundTime++;
        }
        //웨이팅큐 업데이트
        if (waitingCount > 0) {
            for (int i = 0; i < waitingCount; i++) {
                waitingQueue[i]->turnaroundTime++;
                waitingQueue[i]->IOremaining--;
                if (waitingQueue[i]->IOremaining <= 0) { //IO 작업 완료
                    printf("pid %d [I/O complete], ", waitingQueue[i]->pid);

                    //레디큐에 삽입, 웨이팅큐에서 삭제
                    readyQueue[readyCount++] = waitingQueue[i];
                    for (int j = i + 1; j < waitingCount; j++) { 
                        waitingQueue[j - 1] = waitingQueue[j];}
                    waitingCount--;
                    i--; 
                }
            }
        }
        //IO 작업 필요 시 처음에 CPU 1만큼 진행 후 웨이팅큐에 삽입
        if(runningProcess != NULL){
            if (runningProcess->IOremaining > 0){ 
            printf("pid %d [I/O request], ", runningProcess->pid);
            waitingQueue[waitingCount++] = runningProcess;
            runningProcess = NULL;}
        }
        
        currentTime++;
        printf("\n");
    }

    EvaluateOne(p);

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(waitingQueue[i]);
    }
    free(waitingQueue);
    for (int i = 0; i < processNum; i++) {
        free(p[i]);
    }
    free(p);
    free(readyQueue);
}

int main(void){
    CreateProcess();

    FCFS();
    NonPreemptive_SJF();
    Preemptive_SJF();
    NonPreemptive_Priority();
    Preemptive_Priority();
    RR();

    EvaluateAll(); //마지막으로 evals 배열의 전체 algorithm 결과 비교

    //메모리 해제
    for (int i = 0; i < processNum; i++) {
        free(processes[i]);
    }
    free(processes);
    for (int i = 0; i < ALGORITHM_NUM; i++) {
        free(evals[i]);
    }

    return 0;
}