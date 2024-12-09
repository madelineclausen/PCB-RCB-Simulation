#include <iostream>
#include <fstream>
#include <queue>
#include <deque>
#include <cstring>
#include <string>

bool process_has_resource(int resource);
bool is_child(int process);
int how_many_resource(int resource);
void clear_pcb();
void clear_rl();
void clear_rcb();
void init();
void scheduler();
void add_child(std::deque<int>& q, int i);
void create(int p);
void request(int resource, int num_units);
void subtract_r(std::deque<struct node> q, int resource, int num_units);
void release(int resource, int num_units);
void timeout();
void release_all_r(int process);
void destroy_all_children(int process);
void destroy(int process_j);
void remove_from_waitlist(int process_j);
void remove_from_rl(int process_j);
void remove_from_parent(int process_j);