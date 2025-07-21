#ifndef MEMBER_THREAD_H
#define MEMBER_THREAD_H

typedef struct {
    int gang_id;
    int member_id;
    int rank;
    int is_agent;
    int preparation_level;
    int last_info_truth;
    int contributions;  // New field to track service

} MemberArgs;

void* member_routine(void* arg);

#endif