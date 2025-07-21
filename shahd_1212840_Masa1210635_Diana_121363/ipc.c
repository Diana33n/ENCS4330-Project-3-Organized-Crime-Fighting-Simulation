#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/stat.h>

SharedState *shared_state = NULL;
int shm_id = -1;

void read_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char key[64];
        int value;
        if (sscanf(line, "%[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "num_gangs") == 0) shared_state->num_gangs = value;
            else if (strcmp(key, "min_members") == 0) shared_state->min_members = value;
            else if (strcmp(key, "max_members") == 0) shared_state->max_members = value;
            else if (strcmp(key, "num_ranks") == 0) shared_state->num_ranks = value;
            else if (strcmp(key, "agent_success_rate") == 0) shared_state->agent_success_rate = value;
            else if (strcmp(key, "suspicion_threshold") == 0) shared_state->suspicion_threshold = value;
            else if (strcmp(key, "info_spread_depth") == 0) shared_state->info_spread_depth = value;
            else if (strcmp(key, "false_spread_rate") == 0) shared_state->false_spread_rate = value;
            else if (strcmp(key, "plan_success_rate") == 0) shared_state->plan_success_rate = value;
            else if (strcmp(key, "prison_time") == 0) shared_state->prison_time = value;
            else if (strcmp(key, "kill_rate") == 0) shared_state->kill_rate = value;
            else if (strcmp(key, "max_thwarted_plans") == 0) shared_state->max_thwarted_plans = value;
            else if (strcmp(key, "max_successful_plans") == 0) shared_state->max_successful_plans = value;
            else if (strcmp(key, "max_executed_agents") == 0) shared_state->max_executed_agents = value;
        }
    }
    fclose(file);

    // Validation
    if (shared_state->min_members < 1) {
        fprintf(stderr, "min_members must be ≥1 (got %d)\n", shared_state->min_members);
        exit(EXIT_FAILURE);
    }
    if (shared_state->max_members < shared_state->min_members) {
        fprintf(stderr, "max_members must be ≥min_members (%d < %d)\n",
               shared_state->max_members, shared_state->min_members);
        exit(EXIT_FAILURE);
    }
}

void init_shared_state() {
    printf("🔧 Initializing shared state (no malloc)...\n");

    for (int g = 0; g < shared_state->num_gangs; g++) {
        shared_state->gang_in_prison[g] = 0;
        shared_state->shared_plan_ready[g] = 0;

        for (int m = 0; m < shared_state->max_members; m++) {
            shared_state->member_status[g][m] = 1;
            shared_state->member_prep_levels[g][m] = 0;
            shared_state->knowledge[g][m] = 0;
          shared_state->  agent_members[g][m] = 0;
            strncpy(shared_state->fake_plans[g][m], "FAKE", MAX_FAKE_PLAN_LEN);
        }

        strncpy(shared_state->shared_plan[g], "NONE", MAX_PLAN_LEN);
    }


    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_state->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    printf("✅ Shared state initialized.\n");
}



void attach_ipc()
 {
    key_t key = 0x1234;
    int attempts = 0;
    const int max_attempts = 5;

    while (attempts < max_attempts) {
        shm_id = shmget(key, sizeof(SharedState), 0666);
        if (shm_id == -1) {
            attempts++;
            if (attempts >= max_attempts) {
                perror("shmget failed in attach_ipc - is main process running?");
                exit(EXIT_FAILURE);
            }
            sleep(1);
            continue;
        }

        shared_state = (SharedState *)shmat(shm_id, NULL, 0);
        if (shared_state == (void *)-1) {
            attempts++;
            if (attempts >= max_attempts) {
                perror("shmat failed in attach_ipc");
                exit(EXIT_FAILURE);
            }
            sleep(1);
            continue;
        }

        // Simple check to confirm the memory was initialized
        if (shared_state->num_gangs > 0) {
            break;
        }

        attempts++;
        if (attempts >= max_attempts) {
            fprintf(stderr, "Shared memory appears uninitialized\n");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
}

void init_ipc(const char *config_file) {
    // Use a fixed key (could also use ftok() for better portability)
    key_t key = 0x1234;
size_t shm_size = sizeof(SharedState);
printf("Attempting to allocate shared memory of size: %zu bytes\n", shm_size);

    printf("Attempting to allocate shared memory of size: %zu bytes\n", shm_size);

    // First try to remove any existing shared memory segment
    int existing_shm = shmget(key, 0, 0);
    if (existing_shm != -1) {
        if (shmctl(existing_shm, IPC_RMID, NULL) == -1) {
            perror("Warning: Failed to remove existing shared memory");
        } else {
            printf("Cleaned up existing shared memory segment\n");
        }
    }

    // Create new shared memory segment with exclusive flag
    shm_id = shmget(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        perror("shmget failed (exclusive create)");

        // Fallback to getting existing segment if exclusive create failed
        shm_id = shmget(key, shm_size, 0666);
        if (shm_id == -1) {
            perror("shmget failed (fallback)");
            exit(EXIT_FAILURE);
        }
        printf("Using existing shared memory segment\n");
    } else {
        printf("Created new shared memory segment (ID: %d)\n", shm_id);
    }

    // Attach shared memory
    shared_state = (SharedState *)shmat(shm_id, NULL, 0);
    if (shared_state == (void *)-1) {
        perror("shmat failed");

        // Clean up if attach failed
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    printf("Initializing shared memory...\n");
    memset(shared_state, 0, shm_size);
read_config(config_file);  // must come BEFORE using values
printf("✅ Config: gangs=%d, max_members=%d\n",
       shared_state->num_gangs, shared_state->max_members);
init_shared_state();       // safe now


    printf("Shared memory initialized successfully\n");
}


void destroy_ipc() {
    if (shared_state) {
        pthread_mutex_destroy(&shared_state->mutex);
        shmdt(shared_state);
    }

    if (shm_id != -1) {
        shmctl(shm_id, IPC_RMID, NULL);
    }
}
