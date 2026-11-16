#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BURSTS 100

// Process Control Block (PCB) yapisi[cite: 2]
typedef struct {
    char pid[16];
    int arrival_time;
    int original_priority;
    int current_priority;

    int bursts[MAX_BURSTS]; // Çift indeksler CPU, tek indeksler I/O[cite: 2]
    int burst_count;
    int current_burst_index;
    int remaining_time;

    // Metrikler icin zaman damgalari[cite: 2]
    int completion_time;
    int first_dispatch_time;
    int waiting_time;

    // Durum takibi
    int state; // 0: NEW, 1: READY, 2: RUNNING, 3: WAITING, 4: TERMINATED
} PCB;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Hatalı argüman kullanımı.\n");
        return 1;
    }

    printf("MiniOS-Sim baslatiliyor...\n");
    for(int i = 0; i < argc; i++) {
        printf("Arg[%d]: %s\n", i, argv[i]);
    }

    return 0;
}