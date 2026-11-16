#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BURSTS 100
#define MAX_PROCESSES 100

// Process Control Block (PCB)[cite: 2]
typedef struct {
    char pid[16];
    int arrival_time;
    int original_priority;
    int current_priority;
    
    int bursts[MAX_BURSTS]; 
    int burst_count;
    int current_burst_index;
    int remaining_time;
    
    int completion_time;
    int first_dispatch_time;
    int waiting_time;
    
    int state; // 0: NEW, 1: READY, 2: RUNNING, 3: WAITING, 4: TERMINATED
} PCB;

// Tum prosesleri tutacagimiz dizi
PCB processes[MAX_PROCESSES];
int process_count = 0;

// Dosyadan is yuku (workload) okuyan fonksiyon[cite: 2]
void read_workload(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Dosya acilamadi: %s\n", filename);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // '#' ile baslayan yorum satirlarini veya bos satirlari atla[cite: 2]
        if (line[0] == '#' || line[0] == '\n') continue;

        PCB *p = &processes[process_count];
        p->burst_count = 0;
        p->state = 0; // Baslangicta hepsi NEW durumunda

        // Satiri bosluklara gore parcala ve PCB'ye kaydet
        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(p->pid, token); // PID

        token = strtok(NULL, " \t\n");
        if (token) p->arrival_time = atoi(token); // Gelis Zamani

        token = strtok(NULL, " \t\n");
        if (token) {
            p->original_priority = atoi(token); // Oncelik
            p->current_priority = p->original_priority;
        }

        // Kalan tum sayilar burst (CPU ve I/O sureleri)
        while ((token = strtok(NULL, " \t\n")) != NULL) {
            p->bursts[p->burst_count++] = atoi(token);
        }
        process_count++;
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    // Arguman olarak en azindan bir dosya adi bekliyoruz
    if (argc < 2) {
        fprintf(stderr, "Kullanim: ./minios <komutlar> <dosya_adi>\n");
        return 1;
    }

    // Gecici olarak son argumani dosya adi kabul edip okuyoruz
    const char *filename = argv[argc - 1];
    read_workload(filename);

    // Okunan verileri ekrana yazdirip kontrol edelim
    printf("MiniOS-Sim is yuku yuklendi. Okunan proses: %d\n", process_count);
    for (int i = 0; i < process_count; i++) {
        printf("Proses: %s | Gelis: %d | Oncelik: %d | Burst Adedi: %d\n", 
               processes[i].pid, processes[i].arrival_time, 
               processes[i].original_priority, processes[i].burst_count);
    }

    return 0;
}