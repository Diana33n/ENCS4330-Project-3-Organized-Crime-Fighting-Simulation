#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <pthread.h>

#define MAX_GANGS 10
#define MAX_MEMBERS 20
#define MAX_INFO_LEN 256
#define MAX_PLAN_LEN 128
#define MAX_FAKE_PLAN_LEN 100

#define MQ_NAME "/agent_to_police"

typedef struct {
    int agent_id;
    int gang_id;
    int suspicion_level;
    char info[MAX_INFO_LEN];
} AgentMessage;

typedef struct {
    // Stats
    int thwarted_plans;
    int successful_plans;
    int executed_agents;
int simulation_done;
    // Per-gang data
    int gang_in_prison[MAX_GANGS];
    int gang_prison_timer[MAX_GANGS];
    int prep_level[MAX_GANGS];
    int thwart_requests[MAX_GANGS];
    char shared_plan[MAX_GANGS][MAX_PLAN_LEN];
    int shared_plan_ready[MAX_GANGS];
    int prep_time_remaining[MAX_GANGS];
    int info_requested[MAX_GANGS];
    int required_prep_level[MAX_GANGS];
    int gang_member_count[MAX_GANGS];
    int current_investigation_phase[MAX_GANGS];
    int investigation_active[MAX_GANGS];
    int investigation_timer[MAX_GANGS];
    int investigation_depth[MAX_GANGS];
    float accumulated_suspicion[MAX_GANGS];

    // Per-member data
    int member_prep_levels[MAX_GANGS][MAX_MEMBERS];
    int knows_real_plan[MAX_GANGS][MAX_MEMBERS];
    int is_under_fake_plan[MAX_GANGS][MAX_MEMBERS];
    int member_ranks[MAX_GANGS][MAX_MEMBERS];
    int member_status[MAX_GANGS][MAX_MEMBERS];  // 0 = dead, 1 = alive
    int catch_agent[MAX_GANGS][MAX_MEMBERS];
    int catcher[MAX_GANGS][MAX_MEMBERS];
    int suspicion[MAX_GANGS][MAX_MEMBERS];
    int knowledge[MAX_GANGS][MAX_MEMBERS];
    int already_investigated[MAX_GANGS][MAX_MEMBERS];
    int end_investigation[MAX_GANGS][MAX_MEMBERS];
    int agents_reported[MAX_GANGS][MAX_MEMBERS];
    char fake_plans[MAX_GANGS][MAX_MEMBERS][100];

    // Config values
    int num_gangs;
    int min_members;
    int max_members;
    int num_ranks;
    int agent_success_rate;
    int suspicion_threshold;
    int info_spread_depth;
    int false_spread_rate;
    int plan_success_rate;
    int prison_time;
    int kill_rate;
    int max_thwarted_plans;
    int max_successful_plans;
    int max_executed_agents;
    int agent_members[MAX_GANGS][MAX_MEMBERS];
    int recent_success[MAX_GANGS];  // Track which gangs recently succeeded
    // Mutex
    pthread_mutex_t mutex;
} SharedState;

#endif
