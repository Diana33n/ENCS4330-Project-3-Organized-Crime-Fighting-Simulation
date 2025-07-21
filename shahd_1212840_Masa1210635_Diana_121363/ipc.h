#ifndef IPC_H
#define IPC_H

#include "shared_state.h"

extern SharedState *shared_state;
extern int shm_id;

void read_config(const char *filename);
void init_ipc(const char *config_file);
void attach_ipc();  // Add this new function
void destroy_ipc();
void init_shared_state();

#endif