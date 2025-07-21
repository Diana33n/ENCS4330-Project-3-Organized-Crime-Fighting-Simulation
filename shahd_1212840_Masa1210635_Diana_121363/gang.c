#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "ipc.h"
#include "member_thread.h"

void* gang_monitor(void* arg) {
    int gang_id = *(int*)arg;
    while (1) {
        if (shared_state->prep_time_remaining[gang_id] > 0) {
            printf("Gang %d preparing... Time left: %d\n",
                   gang_id, shared_state->prep_time_remaining[gang_id]);
        }

        if (shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
            shared_state->successful_plans >= shared_state->max_successful_plans ||
            shared_state->executed_agents >= shared_state->max_executed_agents) {
            printf("📣 Gang monitor %d exiting...\n", gang_id);
            break;
        }

        usleep(100000);
    }
    return NULL;
}

void replace_dead_members(int gang_id) {
    pthread_mutex_lock(&shared_state->mutex);

    if (shared_state->prep_time_remaining[gang_id] > 0&&shared_state->shared_plan_ready[gang_id]) {
        for (int i = 1; i < shared_state->gang_member_count[gang_id]; i++) {
            if (shared_state->member_status[gang_id][i] == 0) {
                MemberArgs *args = malloc(sizeof(MemberArgs));
                args->gang_id = gang_id;
                args->member_id = i;
                args->is_agent = (rand() % 100 < shared_state->agent_success_rate);
                args->rank = 0;
                shared_state->member_ranks[gang_id][i] = args->rank;
                args->preparation_level = 0;
                args->contributions = 0;
                args->last_info_truth = 0;

                pthread_t thread;
                pthread_create(&thread, NULL, member_routine, args);

                shared_state->member_status[gang_id][i] = 1;
                printf("➕ Gang %d replaced member %d (New recruit in same position)\n", gang_id, i);
            }
        }
    }

    pthread_mutex_unlock(&shared_state->mutex);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <gang_id>\n", argv[0]);
        return 1;
    }

    attach_ipc("config.txt");

    if (!shared_state) {
        fprintf(stderr, "Failed to access shared memory\n");
        return 1;
    }

    int gang_id = atoi(argv[1]);
    srand(time(NULL) + gang_id);

    int member_count = shared_state->min_members;
    if (shared_state->max_members > shared_state->min_members) {
        member_count += rand() % (shared_state->max_members - shared_state->min_members + 1);
    }
    member_count = member_count < 1 ? 1 : member_count;
    shared_state->gang_member_count[gang_id] = member_count;

    printf("🚩 Gang %d launching with %d members\n", gang_id, member_count);

    pthread_t threads[member_count];
    pthread_t monitor_thread;

    pthread_create(&monitor_thread, NULL, gang_monitor, &gang_id);

    for (int i = 0; i < member_count; i++) {
        shared_state->member_status[gang_id][i] = 1;
    }

    for (int i = 0; i < member_count; i++) {
        MemberArgs *args = malloc(sizeof(MemberArgs));
        args->gang_id = gang_id;
        args->member_id = i;
         if(i>0){
            args->is_agent = (rand() % 100 < shared_state->agent_success_rate);
            if(i==(member_count-2)){
            args->is_agent=1;
            }
        }
        else{
        args->is_agent=0;
        }

        args->rank = (i == 0) ? shared_state->num_ranks - 1 : (rand() % 7);
        shared_state->member_ranks[gang_id][i] = args->rank;
        args->preparation_level = 0;
        args->contributions = (i == 0) ? 100 : 0;
        args->last_info_truth = 0;
        shared_state->agent_members[gang_id][i] = args->is_agent;
        pthread_create(&threads[i], NULL, member_routine, args);
    }

    while (1) {
        if (shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
            shared_state->successful_plans >= shared_state->max_successful_plans ||
            shared_state->executed_agents >= shared_state->max_executed_agents) {
            printf("📣 Gang %d main loop exiting...\n", gang_id);
            break;
        }

        replace_dead_members(gang_id);
        usleep(1000000);
    }

    for (int i = 0; i < member_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_cancel(monitor_thread);
    pthread_join(monitor_thread, NULL);

    return 0;
}
