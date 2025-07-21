

# 🕵️ Organized Crime-Fighting Simulation

A multi-processing, multi-threaded simulation of undercover agents infiltrating organized crime gangs. Designed as part of:

> 📘 **ENCS4330 – Real-Time Applications & Embedded Systems**
> 📅 *2nd Semester 2024/2025*
> 👨‍🏫 Instructor: Dr. Hanna Bullata

This system models dynamic gang activities, secret agent infiltration, and police intervention—leveraging **POSIX threads**, **shared memory**, **semaphores**, and **message queues** for coordination. 🧠

---

## 🎯 Objectives

* 🏢 Simulate multiple gangs with hierarchical structures (ranks).
* 🕵️ Plant secret agents to gather intelligence covertly.
* 👮 Police intercept gang plans based on agent reports.
* 🖥️ Visualize gang activities and police interventions using OpenGL.
* 💣 Handle dynamic events: promotions, false information spread, investigations, imprisonments, and assassinations.

---


## 📦 Features

### 🏢 Gangs

* Multiple gangs (user-defined in `config.txt`)
* Each gang has members with hierarchical ranks.
* Bosses plan crimes and assign preparation goals.
* Information is shared hierarchically (higher ranks know more).
* Bosses may spread **false plans** to confuse agents.

### 🕵️ Secret Agents

* Agents infiltrate gangs (insertion rate is user-defined).
* Behave like regular members and climb ranks to access sensitive info.
* Gather intelligence cautiously to avoid suspicion.
* Send reports to police when suspicion or knowledge threshold is reached.

### 👮 Police

* Receives agent reports via POSIX message queues (`/agent_to_police`).
* Calculates accumulated suspicion per gang.
* Arrests gangs if suspicion exceeds thresholds.
* Handles prison timers and releases gangs after user-defined periods.

### 🎭 Dynamic Events

* Promotions for loyal gang members.
* Random member deaths during missions.
* Internal investigations by bosses to uncover agents.
* Execution of uncovered agents.

### 🎨 Visualization

* OpenGL display of:

  * Gangs and their members
  * Active plans and statuses
  * Arrests and executions
  * Police interventions

---



## 🚀 Build & Run

### 🏗️ Build

```bash
make
```

### ▶ Run Simulation

```bash
./main config.txt
```

---

## 📊 Termination Conditions

The simulation ends if:
✅ Police thwarts more than `max_thwarted_plans` crimes.

✅ Gangs succeed in more than `max_successful_plans` crimes.

✅ More than `max_executed_agents` agents are uncovered and executed.

---

## 👩‍💻 Technologies Used

* 🧵 POSIX Threads (`pthread_create`, `pthread_mutex`)
* 🛠 Shared Memory (`shmget`, `shmat`)
* 🔐 Semaphores for resource locking
* 📩 POSIX Message Queues (`mq_open`, `mq_send`, `mq_receive`)
* 🎨 OpenGL (GLUT) for visualization
* ⏳ Timers for prison durations and investigation phases

---

## 👥 Team Members

| Name           | Student ID |
| -------------- | ---------- |
| Diana Muzahem  | 1210363    |
| Shahd Abdallah | 1212840    |
| Masa Shaheen   | 1210635    |

---
