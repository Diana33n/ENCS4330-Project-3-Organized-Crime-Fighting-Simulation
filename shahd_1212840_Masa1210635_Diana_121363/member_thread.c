//Shahd Abdallah 1212840
//Masa Shaheen 1210635
//Diana Muzahem 1210363
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "ipc.h"
#include "member_thread.h"
#include <mqueue.h>
#include <fcntl.h>

#define MAX_KNOWLEDGE 100
#define MAX_TARGET_LEN 128
#include <sys/ipc.h>
#include <sys/msg.h>
int used_indices[7] = {0};  // tracks which fake targets were used
int candidates[10];
struct msgbuf {
    long mtype;
    char mtext[100];
};
int investigation_round=0;
const char* targets[] = {
    "Robbing banks and financial institutions",
    "Robbing gold/jewelry shops",
    "Drug trafficking (buying and selling)",
    "Robbing expensive artwork",
    "Kidnapping wealthy people for ransom",
    "Blackmailing wealthy individuals",
    "Arms trafficking"
};
const char* fake_targets[] = {
    "Intercepting a weapons drop at the docks of Port Zahran at midnight",
    "Planting a malware chip in the central server room of Eastbank Telecom HQ",
    "Coordinating a fake hostage crisis in the Royal Grand Hotel lobby",
    "Tampering with armored cash trucks on Route 22 between Hebron and Jericho",
    "Sabotaging surveillance equipment in the Metro Line C control center",
    "Breaking into the customs vault at Terminal 3 of Al-Salam Airport",
    "Disrupting satellite uplinks from the Ministry of Defense relay station"
};


const char* fake_mission = "Distraction Operation Alpha";

// Boss chooses and broadcasts the plan
void boss_plan_target(int gang_id) {
    int index = rand() % 7;
    strcpy(shared_state->shared_plan[gang_id], targets[index]);
    shared_state->shared_plan_ready[gang_id] = 1;
    shared_state->required_prep_level[gang_id] = 20 + (rand() % 21);

    printf("🧠 Boss of Gang %d selected target: '%s'\n", gang_id, shared_state->shared_plan[gang_id]);
}
void boss_investigation(int gang_id) {
    int top_rank = shared_state->num_ranks - shared_state->investigation_depth[gang_id];

    if (top_rank < 0) {
        printf("🛑 Reached max investigation depth for Gang %d. Ending investigation.\n", gang_id);
        shared_state->investigation_active[gang_id] = 0;
        shared_state->investigation_depth[gang_id] = 0;
        shared_state->investigation_timer[gang_id] = 0;
        shared_state->current_investigation_phase[gang_id] = 0;
        return;
    }

    if (top_rank < 3) top_rank = 3;

    printf("🔍 Entered boss_investigation() for Gang %d | Top Rank: %d\n", gang_id, top_rank);

    
    if (shared_state->current_investigation_phase[gang_id] == 1) {
        shared_state->investigation_timer[gang_id]--;
        printf("⏳ Waiting for agent leaks... Timer:%d for gang %d\n", shared_state->investigation_timer[gang_id], gang_id);
       
            int agent_found = 0;
            for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
                if (shared_state->is_under_fake_plan[gang_id][i] &&
                    shared_state->catch_agent[gang_id][i]) {

                    shared_state->knowledge[gang_id][i] = 0;
                    shared_state->suspicion[gang_id][i] = 0;
                    shared_state->is_under_fake_plan[gang_id][i] = 0;
                 printf("🔫member%d from gang %d the before killed ones:%d🔫\n",i,gang_id,shared_state->executed_agents);

                    shared_state->executed_agents++;
                    printf("🔫member%d from gang %d the killed ones:%d🔫\n",i,gang_id,shared_state->executed_agents);
                    agent_found = 1;
                    shared_state->member_status[gang_id][i] = 0; // Mark as dead


                    printf("⚠️🔫⚠️🔫⚠️🔫⚠️🔫⚠️🔫⚠️🔫 Agent %d exposed and executed.⚠️🔫⚠️🔫⚠️🔫⚠️🔫⚠️🔫⚠️🔫\n", i);
                                    shared_state->investigation_active[gang_id] = 0;

                }
            }

        
        if (shared_state->investigation_timer[gang_id] == 0) {
            printf("⏰ Time is up! Evaluating investigation results...for gang %d\n", gang_id);


            if (!agent_found) {
                shared_state->investigation_depth[gang_id]++;
                shared_state->investigation_timer[gang_id] = 6;
                printf("🔁 No agents found. Going deeper to depth %d...\n", shared_state->investigation_depth[gang_id]);
            } else {
                shared_state->investigation_active[gang_id] = 0;
                shared_state->investigation_depth[gang_id] = 0;
                shared_state->investigation_timer[gang_id] = 0;
                printf("✅ Agent found! Investigation ended for Gang %d.\n", gang_id);
                              printf("👑👑👑👑 Boss of Gang %d is active %d inside 👑👑👑.\n", gang_id,shared_state->investigation_active[gang_id]);

            }

            shared_state->current_investigation_phase[gang_id] = 0;
        }

        return; // Don't proceed to new investigation yet
    }
    
    int candidate_count = 0;
    for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
        printf("👁️ Member %d|Gang %d |is agent %s | Rank=%d | knows_real=%d | already_investigated=%d | fake_plan=%d\n",
               i,gang_id,shared_state->agent_members[gang_id][i] ? "[AGENT]" : "[NORMAL]", 
               shared_state->member_ranks[gang_id][i],
               shared_state->knows_real_plan[gang_id][i],
               shared_state->already_investigated[gang_id][i],
               shared_state->is_under_fake_plan[gang_id][i]);

        if (shared_state->member_ranks[gang_id][i] >= top_rank &&
           
            !shared_state->is_under_fake_plan[gang_id][i] &&
            !shared_state->already_investigated[gang_id][i] &&
            i != 0&&shared_state->member_status[gang_id][i]) {

            candidates[candidate_count++] = i;
            printf("🕵️ Member %d selected for investigation\n", i);
        }
    }

   if (candidate_count == 0) {
    printf("⚠️ No candidates found at depth %d. Moving immediately to deeper investigation...\n", shared_state->investigation_depth[gang_id]);

    shared_state->investigation_depth[gang_id]++;
    shared_state->investigation_timer[gang_id] = 6;  // reset timer for next round

    if (shared_state->investigation_depth[gang_id] >= shared_state->num_ranks) {
        printf("🛑 Max depth reached for Gang %d. Ending investigation.\n", gang_id);
        shared_state->investigation_active[gang_id] = 0;
        shared_state->investigation_depth[gang_id] = 0;
        shared_state->investigation_timer[gang_id] = 0;

    }

    return; // skip this round, retry at next level on next call
}



    int to_investigate = candidate_count < 2 ? candidate_count : 2;
    printf("🔎 Investigating %d members this round\n", to_investigate);

    for (int j = 0; j < to_investigate; j++) {
        int i = candidates[j];
        shared_state->already_investigated[gang_id][i] = 1;
        shared_state->end_investigation[gang_id][i] = 1;
        shared_state->catcher[gang_id][i] = 1;

        int fake_index, attempts = 0;
        do {
            fake_index = rand() % 7;
            attempts++;
        } while (used_indices[fake_index] && attempts < 10);

        used_indices[fake_index] = 1;
        strcpy(shared_state->fake_plans[gang_id][i], fake_targets[fake_index]);
        shared_state->is_under_fake_plan[gang_id][i] = 1;
        
        printf("🎭 Boss gave fake plan to Member %d: '%s'\n", i, shared_state->fake_plans[gang_id][i]);
        shared_state->knowledge[gang_id][i] = 100;
        shared_state->suspicion[gang_id][i] = 500;
         printf("👤 Member %d (Gang %d) | Knowledge: %d | Suspicion: %d\n",
            i, gang_id, shared_state->knowledge[gang_id][i] ,  shared_state->suspicion[gang_id][i]);


    }

    // Enter phase 1: wait for agent to report
    shared_state->investigation_timer[gang_id] = 6;
    shared_state->current_investigation_phase[gang_id] = 1;
}


void* member_routine(void* arg) {
    MemberArgs* args = (MemberArgs*)arg;
    

    int is_boss = (args->member_id == 0);
    int gang_id = args->gang_id;
    int member_id = args->member_id;

    printf("👤 Member %d from Gang %d started. %s Rank: %d\n",
           member_id, gang_id,
           args->is_agent ? "[AGENT]" : "[NORMAL]", args->rank);
 while (1) {

pthread_mutex_lock(&shared_state->mutex);
if (shared_state->successful_plans >= shared_state->max_successful_plans ||
    shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
    shared_state->executed_agents >= shared_state->max_executed_agents) {
    printf("🛑 Member %d from Gang %d exiting: simulation complete.\n", member_id, gang_id);
    pthread_mutex_unlock(&shared_state->mutex);  // ✅ now safe
    break;
}
pthread_mutex_unlock(&shared_state->mutex);  // If not breaking, continue normal flow

printf("👤 Member %d from Gang %d | rank: %d | contribution: %d\n",member_id, gang_id,shared_state->member_ranks[gang_id][member_id],args->contributions);
    
 printf("👤 Member %d from Gang %d started. %s \n",
           member_id, gang_id,
          args->is_agent ? "[AGENT]" : "[NORMAL]");
          shared_state->agent_members[gang_id][member_id] = args->is_agent;

        shared_state->member_ranks[gang_id][member_id] = args->rank;
       
      if (shared_state->member_status[gang_id][member_id] == 0) {
             printf("💀 Member %d from Gang %d is marked executed. Exiting...\n", member_id, gang_id);

        break;
       }

       
if (shared_state->gang_in_prison[gang_id]) {
    usleep(500000);
    printf("🚔 Gang %d is in prison. Member %d waiting...\n", gang_id, member_id);
    continue;
}



        pthread_mutex_lock(&shared_state->mutex);

        /* ========== BOSS LOGIC ========== */
        if (is_boss) {
           printf("👑 Boss of Gang %d is active %d.\n", gang_id,shared_state->investigation_active[gang_id]);
        
            if (shared_state->investigation_active[gang_id]&&!shared_state->gang_in_prison[gang_id]) {
                printf("🔍 Boss of Gang %d is investigating! No new plans issued.\n", gang_id);
                boss_investigation(gang_id);
                pthread_mutex_unlock(&shared_state->mutex);
                usleep(500000);
                continue;
            }               
          
            // Initialize new plan if none exists
            if (!shared_state->shared_plan_ready[gang_id]) {
                // Select crime and set preparation time (10-20 cycles)
                shared_state->prep_time_remaining[gang_id] = 10 + (rand() % 11);
                boss_plan_target(gang_id);//choose mission

                // Reset all member preparation levels
                for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
                    shared_state->member_prep_levels[gang_id][i] = 0;
                }

                printf("🧠🧠🧠🧠🧠🧠🧠🧠🧠🧠🧠🧠🧠🧠 BOSS OF GANG  %d\n",gang_id );
                printf("🧠 Boss of Gang %d selected: '%s' (Time: %d cycles)\n",
                      gang_id, shared_state->shared_plan[gang_id],
                      shared_state->prep_time_remaining[gang_id]);
            }

            // Decrement preparation timer
                if (shared_state->shared_plan_ready[gang_id]) {
                    shared_state->prep_time_remaining[gang_id]--;
                    if (shared_state->prep_time_remaining[gang_id] < 0) {
                        shared_state->prep_time_remaining[gang_id] = 0;
                    }
                }


          if (shared_state->shared_plan_ready[gang_id]) {
    int all_ready = 1;
    for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
        if (shared_state->member_prep_levels[gang_id][i] < shared_state->required_prep_level[gang_id]) {
            all_ready = 0;
            break;
        }
    }



    if (all_ready) {
        int sum = 0;
        for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
            sum += shared_state->member_prep_levels[gang_id][i];
        }

        int avg = sum / shared_state->gang_member_count[gang_id];
        int success_chance = shared_state->plan_success_rate + (avg / 2);
        int roll = rand() % 100;

        printf("\n🎯 Boss of Gang %d executing: '%s'\n", gang_id, shared_state->shared_plan[gang_id]);
        printf("📊 Avg Prep = %d | Success Chance = %d%% | Roll = %d\n",
            avg, success_chance, roll);

        if (roll <= success_chance) {
            shared_state->successful_plans += 1;
            shared_state->recent_success[gang_id] = 10; // Set to number of frames to show success
            printf("✅ SUCCESS! Gang %d completed their mission!\n\n", gang_id);

                        for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
                            if (!is_boss) {
                            args->contributions += 10 + (rand() % 21); // 10-30 points instead of fixed 15
                            }
                        }

        } else {
            shared_state->thwarted_plans += 1;
            printf("❌ FAILURE! Gang %d's mission was stopped!\n\n", gang_id);
            shared_state->investigation_active[gang_id] = 1;
            shared_state->investigation_depth[gang_id] = shared_state->info_spread_depth;
            shared_state->investigation_timer[gang_id] = 6;
        }
        // Random risk: some members die during the mission
        for (int i = 1; i < shared_state->gang_member_count[gang_id]; i++) { // skip boss (id 0)
            if (shared_state->member_status[gang_id][i] == 1 &&
                (rand() % 100) < shared_state->kill_rate) {

                shared_state->member_status[gang_id][i] = 0;
                printf("💥💀 Member %d from Gang %d died during the mission!\n", i, gang_id);
            }
        }


        
        
        // Reset for next mission
        shared_state->shared_plan_ready[gang_id] = 0;
        for (int i = 1; i < shared_state->gang_member_count[gang_id]; i++) {
            shared_state->suspicion[gang_id][i] = 0; // Reset suspicion for next mission
            shared_state->knowledge[gang_id][i] = 0; // Reset knowledge for next mission
        }

    }
}

        }
         // Info sharing// MAKE IT WHEN THE PLAN IS READY
        if (shared_state->prep_time_remaining[gang_id] % 3 == 0&&! shared_state->investigation_active[gang_id]) {
            int top_rank = shared_state->num_ranks - shared_state->info_spread_depth;// to shm

            if (args->rank >= top_rank &&
                shared_state->shared_plan_ready[gang_id]) {

                // High-rank gets real info from boss
                args->last_info_truth = 1;
                printf("🔓 Member %d (Rank %d) received REAL info: '%s'\n",
                       member_id, args->rank, shared_state->shared_plan[gang_id]);

                shared_state->knows_real_plan[gang_id][member_id] = 1;  // mark this member
                args->preparation_level += 10;
                 args->contributions += 2 + (rand() % 10); // 2-5 points instead of fixed 3-5

                if (args->is_agent) {
                    shared_state->knowledge[gang_id][member_id] += 20;
                     shared_state->suspicion[gang_id][member_id] += 20;
                }

            } else if (args->rank < top_rank) {
                // Low-rank: ask a real high-rank member (if exists)
                int source_id = -1;
                for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
                    if (shared_state->knows_real_plan[gang_id][i]) {
                        source_id = i;
                        break;  // Pick first one (can randomize later)
                    }
                }

                if (source_id != -1) {
                    int chance = rand() % 100;
                    int receives_truth = (chance >= shared_state->false_spread_rate);

                    if (receives_truth) {
                        args->last_info_truth = 1;
                        printf("🧾 Member %d (Rank %d) received TRUE info from Member %d: '%s'\n",
                               member_id, args->rank, source_id,
                               shared_state->shared_plan[gang_id]);
                        args->preparation_level += 10;
                        args->contributions += 2 + (rand() % 14); // 2-13 points instead of fixed 3-5

                        if (args->is_agent) {
                            shared_state->knowledge[gang_id][member_id] += 20;
                             shared_state->suspicion[gang_id][member_id] += 20;
                        }
                    } else {
                        args->last_info_truth = 0;
                        printf("🎭 Member %d (Rank %d) received FALSE info from Member %d: '%s'\n",
                               member_id, args->rank, source_id, fake_mission);
                        args->preparation_level += 5;
                        if (args->is_agent) {
                            shared_state->knowledge[gang_id][member_id] -= 10;
                              shared_state->suspicion[gang_id][member_id] -= 10;
                            if (shared_state->knowledge[gang_id][member_id] < 0) shared_state->knowledge[gang_id][member_id] = 0;
                            if (  shared_state->suspicion[gang_id][member_id] < 0)   shared_state->suspicion[gang_id][member_id] = 0;
                        }
                    }
                } else {
                    printf("⚠️ Member %d couldn't find high-rank source yet.\n", member_id);
                }
            }

        }
    /* ========== PREPARATION WORK ========== */
            shared_state->member_prep_levels[gang_id][member_id] += 5 + (args->rank * 2);
            if (shared_state->member_prep_levels[gang_id][member_id] > 100) {
                shared_state->member_prep_levels[gang_id][member_id] = 100;
            }
                    args->contributions += 1 + (rand() % 5); // 1-5 points instead of fixed 1


                   
                /* ========== Agent logic ========== */

         if (args->is_agent) {
            for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
                if (i == member_id) continue;

                if ( args->rank >= shared_state->member_ranks[gang_id][i] ) {
                    if (shared_state->knows_real_plan[gang_id][i]) {
                        int is_truthful = (rand() % 100) >= shared_state->false_spread_rate;

                        if (is_truthful) {
                            shared_state->knowledge[gang_id][member_id] += 20;
                            printf("🕵️ Agent -> Member %d (Rank %d) gained knowledge from member %d (Rank %d) (+20)\n",
                                   member_id, args->rank, i, shared_state->member_ranks[gang_id][i]);
                        } else {
                            shared_state->knowledge[gang_id][member_id] -= 10;
                            printf("🕵️ Agent -> Member %d (Rank %d) misled by member %d (Rank %d) (-10)\n",
                                   member_id, args->rank, i, shared_state->member_ranks[gang_id][i]);
                        }

                        if (shared_state->knowledge[gang_id][member_id] > 100) shared_state->knowledge[gang_id][member_id] = 100;
                        if (shared_state->knowledge[gang_id][member_id] < 0) shared_state->knowledge[gang_id][member_id] = 0;

                        shared_state->suspicion[gang_id][member_id] += 5;
                        if (shared_state->suspicion[gang_id][member_id] > 100) shared_state->suspicion[gang_id][member_id] = 100;
                        break;
                    }
                }
            }
        }
            
             /* ========== Reportting ========== */
if (args->is_agent) {
    // Before sending report
if (
    shared_state->catcher[gang_id][member_id] &&
    shared_state->suspicion[gang_id][member_id] >= shared_state->suspicion_threshold && shared_state->knowledge[gang_id][member_id] >= 100) {
     shared_state->catch_agent[gang_id][member_id] = 1;
    printf("🕵️ Agent has catched by the boss\n");
    printf("🕵️ Agent %d from Gang %d has been caught leaking the FAKE plan given during investigation.\n",
           member_id, gang_id);
            shared_state->suspicion[gang_id][member_id] = 0;
        shared_state->knowledge[gang_id][member_id] = 0;
}
    if (shared_state->suspicion[gang_id][member_id]  < shared_state->suspicion_threshold) {
        // Confirmation logic
        int confirmed = 0;
        for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
            if (i != member_id && shared_state->knows_real_plan[gang_id][i]) {
                confirmed = 1;
                shared_state->knowledge[gang_id][member_id] += 15;
                shared_state->suspicion[gang_id][member_id]  += 20;
                printf("🔍 Agent %d confirmed info from Member %d: Suspicion = %d, Knowledge = %d\n",
                       member_id, i, shared_state->suspicion[gang_id][member_id] , shared_state->knowledge[gang_id][member_id]);
                break;
            }
        }
        if (!confirmed) {
            printf("🕵️ Agent %d couldn't confirm any info.\n", member_id);
        }
    }
printf("🔎 Agent %d checking send condition... Suspicion: %d, Knowledge: %d, Threshold: %d\n",
       member_id, shared_state->suspicion[gang_id][member_id] , shared_state->knowledge[gang_id][member_id], shared_state->suspicion_threshold);

    if (shared_state->suspicion[gang_id][member_id] >= shared_state->suspicion_threshold && shared_state->knowledge[gang_id][member_id] > 100&&!shared_state->catcher[gang_id][member_id] ) {
    
       printf("📬 Agent %d attempting to send report...\n", member_id);





// Send the report
AgentMessage msg;
msg.agent_id = member_id;
msg.gang_id = gang_id;
msg.suspicion_level = shared_state->suspicion[gang_id][member_id];
strncpy(msg.info, shared_state->shared_plan[gang_id], MAX_INFO_LEN);


        mqd_t mq = mq_open(MQ_NAME, O_WRONLY);
if (mq != -1) {
    if (mq_send(mq, (char *)&msg, sizeof(AgentMessage), 0) == 0) {
        printf("✅ Agent %d sent report to police. Suspicion = %d\n", member_id, shared_state->suspicion[gang_id][member_id] );
    } else {
        perror("❌ mq_send failed");
    }
    mq_close(mq);
} else {
    perror("❌ mq_open failed (agent)");
}

        shared_state->suspicion[gang_id][member_id] = 0;
        shared_state->knowledge[gang_id][member_id] = 0;
         // Reset for next mission
        shared_state->shared_plan_ready[gang_id] = 0;
    }


    } 


        
       // Promotion logic
        if (!is_boss && shared_state->prep_time_remaining[gang_id] % 5 == 0 &&   args->contributions >= 20 &&
            args->rank < shared_state->num_ranks - 1) {
             args->rank++;
             printf("🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼🔼\n");
            shared_state->member_ranks[gang_id][member_id] = args->rank;
            args->contributions = 0;  // Reset contributions after promotion
            printf("🔼 Member %d promoted to Rank %d for devoted service\n",
                   member_id, args->rank);
        
        }


     pthread_mutex_unlock(&shared_state->mutex);
        usleep(500000); // 0.5s delay between actions
    }

    free(args);
    return NULL;
}