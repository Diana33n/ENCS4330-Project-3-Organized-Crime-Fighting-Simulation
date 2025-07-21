#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ipc.h"
#include "visualization.h"  // or whatever the header file is called

#define MAX_GANGS 10

pid_t gang_pids[MAX_GANGS];
pid_t police_pid;

void* visualization_thread() {
    int argc = 1;
    char *argv[] = {"simulation", NULL};
    initVisualization(argc, argv);
    runVisualization();
    return NULL;
}

void create_gangs() {
    for (int i = 0; i < shared_state->num_gangs; i++) {
        gang_pids[i] = fork();
        if (gang_pids[i] == 0) {
            // Child process - gang
            char gang_id[10];
            snprintf(gang_id, sizeof(gang_id), "%d", i);
            execl("./gang", "gang", gang_id, NULL);
            perror("execl failed");
            exit(EXIT_FAILURE);
        } else if (gang_pids[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        printf("✅ Gang %d launched (PID: %d)\n", i, gang_pids[i]);
    }
}

void create_police() {
    police_pid = fork();
    if (police_pid == 0) {
        // Child process - police
        execl("./police", "police", NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else if (police_pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    printf("🚓 Police process launched (PID: %d)\n", police_pid);
}

void monitor_simulation() {
    while (1) {
        sleep(1);
        printf("📊 Status - Thwarted: %d | Successful: %d | Executed agents: %d\n",
               shared_state->thwarted_plans,
               shared_state->successful_plans,
               shared_state->executed_agents);

        if (shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
            shared_state->successful_plans >= shared_state->max_successful_plans ||
            shared_state->executed_agents >= shared_state->max_executed_agents) {
            printf("🛑 Termination condition met\n");
            break;
        }
    }
}

void cleanup() {
    printf("🧹 Cleaning up simulation...\n");

    // Wait for gangs to exit
    for (int i = 0; i < shared_state->num_gangs && i < MAX_GANGS; i++) {
        if (gang_pids[i] > 0) {
            waitpid(gang_pids[i], NULL, 0);  // ننتظر الخيوط تنتهي طبيعيًا
        }
    }

    // Wait for police
    if (police_pid > 0) {
        waitpid(police_pid, NULL, 0);
    }

    destroy_ipc();
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s config.txt\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("🔧 Initializing Organized Crime Simulation...\n");

    // Initialize shared memory and read config
   init_ipc(argv[1]);

    pthread_t vis_thread;
    pthread_create(&vis_thread, NULL, visualization_thread, NULL);
    printf("Config: %d gangs, %d-%d members each\n",
           shared_state->num_gangs,
           shared_state->min_members,
           shared_state->max_members);
   sleep(3);

    // Create gangs and police
    create_gangs();
    create_police();

    // Monitor simulation
    monitor_simulation();

    // Cleanup
    cleanup();
    printf("🏁 Simulation complete. All processes cleaned up.\n");


    return 0;
}