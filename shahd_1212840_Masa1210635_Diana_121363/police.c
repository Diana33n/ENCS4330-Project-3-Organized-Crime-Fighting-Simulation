#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "ipc.h"
#include "shared_state.h"



void prison_timer_tick() {
    pthread_mutex_lock(&shared_state->mutex);

    for (int i = 0; i < shared_state->num_gangs; i++) {
        if (shared_state->gang_in_prison[i]) {
            if (shared_state->gang_prison_timer[i] > 0) {
                shared_state->gang_prison_timer[i]--;

                if (shared_state->gang_prison_timer[i] == 1) {
                    printf("⚠️ Gang %d will be released in 1 second!\n", i);
                }

                printf("⏳ Gang %d remaining prison time: %d\n",
                       i, shared_state->gang_prison_timer[i]);
            }

            if (shared_state->gang_prison_timer[i] == 0) {
                shared_state->gang_in_prison[i] = 0;
                printf("🔓 Gang %d released from prison!\n", i);
            }
        }
    }

    pthread_mutex_unlock(&shared_state->mutex);
}


int main() {
    attach_ipc("config.txt");
    
    if (!shared_state) {
        fprintf(stderr, "❌ Failed to access shared memory\n");
        return 1;
    }
    
    srand(time(NULL));
    mq_unlink(MQ_NAME);  
    
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(AgentMessage);
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == -1) {
        perror("❌ mq_open (police)");
        return 1;
    } else {
        printf("✅ Police successfully opened message queue\n");
    }

    // pthread_t prison_thread;
    // if (pthread_create(&prison_thread, NULL, prison_timer_thread, NULL) != 0) {
    //     perror("❌ Failed to create prison timer thread");
    //     return 1;
    // }

    printf("🚓 Police process listening...\n");

    while (1) {
        if (shared_state->successful_plans >= shared_state->max_successful_plans ||
            shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
            shared_state->executed_agents >= shared_state->max_executed_agents) {
            printf("🚓 Police exiting: simulation complete.\n");
            break;
        }
         prison_timer_tick(); // <- simplified replacement
    usleep(1000000);     

        AgentMessage msg;
        ssize_t bytes_read = mq_receive(mq, (char *)&msg, sizeof(msg), NULL);
        printf("👮 Police checking message queue...\n");

        if (bytes_read >= 0) {
            if (msg.agent_id == -1 && strcmp(msg.info, "RELEASE_WARNING") == 0) {
                printf("📢 System Warning: Gang %d will be released soon!\n", msg.gang_id);
                continue;
            }

            printf("📨 Received report from Agent %d (Gang %d): %d - %s\n",
                   msg.agent_id, msg.gang_id, msg.suspicion_level, msg.info);

            if (msg.gang_id < 0 || msg.gang_id >= shared_state->num_gangs) {
                printf("❗ Invalid gang_id %d, ignoring...\n", msg.gang_id);
                continue;
            }

            pthread_mutex_lock(&shared_state->mutex);

            if (shared_state->gang_in_prison[msg.gang_id]) {
                printf("⚠️ Gang %d is already in prison. Skipping.\n", msg.gang_id);
                pthread_mutex_unlock(&shared_state->mutex);
                continue;
            }

            int agent_local_id = msg.agent_id % shared_state->max_members;

            if (!shared_state->agents_reported[msg.gang_id][agent_local_id]) {
                shared_state->agents_reported[msg.gang_id][agent_local_id] = 1;
                shared_state->accumulated_suspicion[msg.gang_id] += msg.suspicion_level;
            }

            printf("🔎 Total Suspicion for Gang %d: %.2f / %d\n",
                   msg.gang_id,
                   shared_state->accumulated_suspicion[msg.gang_id],
                   shared_state->suspicion_threshold);

            if (shared_state->accumulated_suspicion[msg.gang_id] >= shared_state->suspicion_threshold) {
                printf("✅ Suspicion threshold met! Arresting Gang %d...\n", msg.gang_id);
                shared_state->gang_in_prison[msg.gang_id] = 1;
                shared_state->gang_prison_timer[msg.gang_id] = shared_state->prison_time;
                shared_state->thwarted_plans++;
                 shared_state->investigation_active[msg.gang_id] = 1;//new investigation
                shared_state->investigation_depth[msg.gang_id] = shared_state->info_spread_depth;//new investigation
                shared_state->investigation_timer[msg.gang_id] = 10;//new investigation
                shared_state->shared_plan_ready[msg.gang_id] = 0;//new investigation

                shared_state->accumulated_suspicion[msg.gang_id] = 0;
                for (int i = 0; i < shared_state->max_members; i++) {
                    shared_state->agents_reported[msg.gang_id][i] = 0;
                }

              
                printf("🚨 Gang %d plan thwarted and sent to prison (%d cycles).\n",
                       msg.gang_id, shared_state->prison_time);
            } else {
                printf("🕵️ Suspicion not enough yet. Waiting for more agents.\n");
            }

            pthread_mutex_unlock(&shared_state->mutex);
        } else if (errno != EAGAIN) {
            perror("❌ mq_receive (police)");
        }

        usleep(500000); // Delay between checks
    }

    mq_close(mq);
    return 0;
}
