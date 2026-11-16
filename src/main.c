#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BURSTS 100
#define MAX_PROCESSES 100

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
    int ready_queue_enter_time;
} PCB;

PCB processes[MAX_PROCESSES];
int process_count = 0;

void read_workload(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Dosya acilamadi: %s\n", filename);
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        PCB *p = &processes[process_count];
        p->burst_count = 0;
        p->state = 0;
        p->first_dispatch_time = -1;
        p->waiting_time = 0;

        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(p->pid, token);
        token = strtok(NULL, " \t\n");
        if (token) p->arrival_time = atoi(token);
        token = strtok(NULL, " \t\n");
        if (token) {
            p->original_priority = atoi(token);
            p->current_priority = p->original_priority;
        }
        while ((token = strtok(NULL, " \t\n")) != NULL) {
            p->bursts[p->burst_count++] = atoi(token);
        }
        p->remaining_time = p->bursts[0];
        p->current_burst_index = 0;
        process_count++;
    }
    fclose(file);
}

// Simulasyon Dongusu (FCFS, SJF, SRTF)
void run_simulation(int print_csv, const char *algo) {
    int tick = 0;
    int completed = 0;
    int current_running = -1;
    int last_gantt_tick = 0;

    if (!print_csv) printf("Gantt Chart (%s): | ", algo);

    while (completed < process_count) {
        // 1. Yeni gelenleri ve I/O bitirenleri READY yap
        for (int i = 0; i < process_count; i++) {
            if (processes[i].state == 0 && processes[i].arrival_time == tick) {
                processes[i].state = 1; 
                processes[i].ready_queue_enter_time = tick;
            } else if (processes[i].state == 3) { 
                processes[i].remaining_time--;
                if (processes[i].remaining_time == 0) {
                    processes[i].current_burst_index++;
                    processes[i].remaining_time = processes[i].bursts[processes[i].current_burst_index];
                    processes[i].state = 1; 
                    processes[i].ready_queue_enter_time = tick;
                }
            }
        }

        // 2. RUNNING durumundaki prosesi islet
        if (current_running != -1) {
            processes[current_running].remaining_time--;
            if (processes[current_running].remaining_time == 0) {
                processes[current_running].current_burst_index++;
                
                if (processes[current_running].current_burst_index >= processes[current_running].burst_count) {
                    processes[current_running].state = 4; // TERMINATED
                    processes[current_running].completion_time = tick;
                    completed++;
                } else {
                    processes[current_running].state = 3; // WAITING
                    processes[current_running].remaining_time = processes[current_running].bursts[processes[current_running].current_burst_index];
                }
                
                if (!print_csv) printf("%s %d-%d | ", processes[current_running].pid, last_gantt_tick, tick);
                last_gantt_tick = tick;
                current_running = -1;
            }
        }

        // 2.5 SRTF Icin Kesme (Preemption) Kontrolu
        if (current_running != -1 && strcmp(algo, "SRTF") == 0) {
            int better_process = -1;
            int shortest_burst = processes[current_running].remaining_time;

            for (int i = 0; i < process_count; i++) {
                if (processes[i].state == 1) { // Sadece READY olanlara bak
                    if (processes[i].remaining_time < shortest_burst) {
                        shortest_burst = processes[i].remaining_time;
                        better_process = i;
                    }
                }
            }
            
            // Eger daha kisa bir is varsa mevcut olani kov
            if (better_process != -1) {
                processes[current_running].state = 1; // Tekrar READY yap
                processes[current_running].ready_queue_enter_time = tick;
                
                if (!print_csv && last_gantt_tick != tick) {
                    printf("%s %d-%d | ", processes[current_running].pid, last_gantt_tick, tick);
                }
                last_gantt_tick = tick;
                current_running = -1; // CPU'yu yeni ise hazirla
            }
        }

        // 3. Algoritmaya Gore CPU'ya Secim Yap
        if (current_running == -1) {
            int selected = -1;
            
            if (strcmp(algo, "FCFS") == 0) {
                int oldest_time = 999999;
                for (int i = 0; i < process_count; i++) {
                    if (processes[i].state == 1) { 
                        if (processes[i].ready_queue_enter_time < oldest_time) {
                            oldest_time = processes[i].ready_queue_enter_time;
                            selected = i;
                        }
                    }
                }
            } 
            else if (strcmp(algo, "SJF") == 0 || strcmp(algo, "SRTF") == 0) {
                int shortest_burst = 999999;
                int oldest_time = 999999;
                for (int i = 0; i < process_count; i++) {
                    if (processes[i].state == 1) {
                        if (processes[i].remaining_time < shortest_burst) {
                            shortest_burst = processes[i].remaining_time;
                            oldest_time = processes[i].ready_queue_enter_time;
                            selected = i;
                        } 
                        else if (processes[i].remaining_time == shortest_burst) {
                            if (processes[i].ready_queue_enter_time < oldest_time) {
                                oldest_time = processes[i].ready_queue_enter_time;
                                selected = i;
                            }
                        }
                    }
                }
            }

            if (selected != -1) {
                if (!print_csv && last_gantt_tick != tick) {
                    printf("IDLE %d-%d | ", last_gantt_tick, tick);
                    last_gantt_tick = tick;
                }
                current_running = selected;
                processes[current_running].state = 2; // RUNNING
                if (processes[current_running].first_dispatch_time == -1) {
                    processes[current_running].first_dispatch_time = tick;
                }
            }
        }

        for (int i = 0; i < process_count; i++) {
            if (processes[i].state == 1) { 
                processes[i].waiting_time++;
            }
        }

        tick++;
    }
    
    if (!print_csv) printf("\n");

    if (print_csv) {
        printf("pid, arrival, completion, turnaround, waiting, response\n");
        for (int i = 0; i < process_count; i++) {
            int turnaround = processes[i].completion_time - processes[i].arrival_time;
            int response = processes[i].first_dispatch_time - processes[i].arrival_time;
            printf("%s,%d,%d,%d,%d,%d\n", 
                   processes[i].pid, 
                   processes[i].arrival_time, 
                   processes[i].completion_time, 
                   turnaround, 
                   processes[i].waiting_time, 
                   response);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Kullanim: ./minios [--csv] [--algo FCFS|SJF|SRTF] <dosya_adi>\n");
        return 1;
    }

    int print_csv = 0;
    char algo[10] = "FCFS"; 
    const char *filename = argv[argc - 1];

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--csv") == 0) {
            print_csv = 1;
        } else if (strcmp(argv[i], "--algo") == 0 && i + 1 < argc - 1) {
            strcpy(algo, argv[i + 1]);
            i++; 
        }
    }

    read_workload(filename);
    run_simulation(print_csv, algo);
    return 0;
}