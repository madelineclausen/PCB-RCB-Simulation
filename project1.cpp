#include "project1.hpp"

#define FREE 0
#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define ALLOCATED 4

struct node
{
    int value1;
    int value2;
};

struct pcb_index
{
    int state = FREE;
    int parent;
    int priority; // 0,1,2
    std::deque<int> children;
    std::deque<struct node> resources;
};

struct rcb_index
{
    int inventory;
    int state;
    std::deque<struct node> waitlist;
};

struct pcb_index PCB[16]; 
struct rcb_index RCB[4];
std::queue<int> RL[3];
int current_level = 0;
int error_detected = false;

int how_many_resource(int resource)
{
    int total = 0;
    std::deque<struct node> q = PCB[RL[current_level].front()].resources;
    for (int i=0; i<q.size(); i++)
    {
        if (resource == q[i].value1)
        {
            total += q[i].value2;
        }
    }
    return total;
}

bool process_has_resource(int resource)
{
    std::deque<struct node> q = PCB[RL[current_level].front()].resources;
    for (int i=0; i<q.size(); i++)
    {
        if (resource == q[i].value1)
        {
            return true;
        }
    }
    return false;
}

bool is_child(int process)
{
    std::deque<int> q = PCB[RL[current_level].front()].children;
    for (int i=0; i<q.size(); i++)
    {
        if (process == q[i])
        {
            return true;
        }
    }
    return false;
}

void clear_pcb()
{
    for (int i=0; i<16; i++)
    {
        pcb_index new_pcb;
        PCB[i] = new_pcb;
    }
}

void clear_rl()
{
    for(int i=0; i<3; i++)
    {
        while(!RL[i].empty()) 
        {
            RL[i].pop();
        }
    }
}

void clear_rcb()
{
    for (int i=0; i<4; i++)
    {
        rcb_index new_rcb;
        new_rcb.inventory = i;
        new_rcb.state = i;
        RCB[i] = new_rcb;
    }
    RCB[0].inventory++;
    RCB[0].state++;
}

void init()
{
    clear_pcb();
    clear_rl();
    clear_rcb();
    pcb_index p_0;
    p_0.state = RUNNING;
    RL[0].push(0);
    PCB[0] = p_0;
}

void scheduler()
{
    if (!RL[2].empty())
    {
        current_level = 2;
    }
    else if (!RL[1].empty())
    {
        current_level = 1;
    }
    else
    {
        current_level = 0;
    }
    PCB[RL[current_level].front()].state = RUNNING;
}

void add_child(std::deque<int>& q, int i)
{
    bool child_added = false;
    for (int j=0; j<q.size(); j++)
    {
        if (i == q[j])
        {
            child_added = true;
        }
    }
    if (!child_added)
    {
        q.push_back(i);
    }
}

void create(int p)
{
    int i = 1;
    while (i<16 && PCB[i].state != FREE)
    {
        i++;
    }
    if ((i==16 && PCB[15].state != FREE) || (p > 2) || (p < 0))
    {
        error_detected = true;
    }
    else
    {
        pcb_index j;
        j.state = READY;
        add_child(PCB[RL[current_level].front()].children, i);
        j.parent = RL[current_level].front();
        j.priority = p;
        RL[p].push(i);
        PCB[i] = j;
        scheduler();
        // printf("process %d created\n", i);
    }
}

void request(int resource, int num_units)
{
    if (resource > 3 || (resource == 0 && num_units != 1) || (resource > 0 && resource < num_units) || (num_units == 0) || RL[current_level].front() == 0)
    {
        error_detected = true;
    }
    else
    {
        if (RCB[resource].state >= num_units)
        {
            RCB[resource].state = RCB[resource].state - num_units;
            if (process_has_resource(resource))
            {
                std::deque<struct node> q = PCB[RL[current_level].front()].resources;
                for (int i=0; i<q.size(); i++)
                {
                    if (q[i].value1 == resource)
                    {
                        q[i].value2 = q[i].value2 + num_units;
                    }
                }

            }
            else
            {
                struct node new_r;
                new_r.value1 = resource;
                new_r.value2 = num_units;
                PCB[RL[current_level].front()].resources.push_back(new_r);
            }
            // printf("resource %d allocated\n", resource);
        }
        else {
            PCB[RL[current_level].front()].state = BLOCKED;
            struct node new_r;
            new_r.value1 = RL[current_level].front();
            new_r.value2 = num_units;
            RCB[resource].waitlist.push_back(new_r);
            // printf("process %d blocked\n", RL.front());
            RL[current_level].pop();
            scheduler();
        }
    }
}

void subtract_r(std::deque<struct node> q, int resource, int num_units)
{
    for (int i=0; i<q.size(); i++)
    {
        if (q[i].value1 == resource)
        {
            q[i].value2 = q[i].value2 - num_units;
            if (q[i].value2 == 0)
            {
                q.erase(q.begin()+i);
            }
        }
    }
}


void release(int resource, int num_units)
{
    if (process_has_resource(resource))
    {
        std::deque<struct node> q = PCB[current_level].resources;
        for (int i=0; i<q.size(); i++)
        {
            if (q[i].value1 == resource)
            {
                num_units = q[i].value2;
                q.erase(q.begin()+i);
            }
        }
        RCB[resource].state = RCB[resource].state + num_units;

        while(!RCB[resource].waitlist.empty() && RCB[resource].state > 0)
        {
            int j = RCB[resource].waitlist[0].value1;
            int k = RCB[resource].waitlist[0].value2;
            if (RCB[resource].state >= k)
            {
                RCB[resource].state = RCB[resource].state - k;
                struct node new_r;
                new_r.value1 = resource;
                new_r.value2 = k;
                PCB[j].resources.push_back(new_r);
                PCB[j].state = READY;    
                RCB[resource].waitlist.erase(RCB[resource].waitlist.begin());
                RL[PCB[j].priority].push(j);
            }
            else
            {
                break;
            }
        }
        scheduler();
    }
    // printf("resource %d released\n", resource);
}

void timeout()
{
    PCB[RL[current_level].front()].state = READY;
    RL[current_level].emplace(RL[current_level].front());
    RL[current_level].pop();
    scheduler();
}

void release_all_r(int process)
{
    std::deque<struct node> q = PCB[process].resources;
    for (int i=0; i<q.size(); i++)
    {
        release(q[i].value1, q[i].value2);
    }
}

void destroy_all_children(int process)
{
    for (int i=0; i<PCB[process].children.size(); i++)
    {
        destroy(PCB[process].children[i]);
    }
}

void remove_from_waitlist(int process_j)
{
    for (int i=0; i<4; i++)
    {
        std::deque<struct node> q = RCB[i].waitlist;
        for (int j=0; j<q.size(); j++)
        {
            if (q[j].value1 == process_j)
            {
                q.erase(q.begin()+j);
            }
        }
    }
}

void remove_from_rl(int process_j)
{
    remove_from_waitlist(process_j);
    for (int i=0; i<3; i++)
    {
        std::queue<int> temp;
        while (!RL[i].empty()) {

            if (RL[i].front() != process_j) 
            {
                temp.push(RL[i].front());
            }
            RL[i].pop();
        }
        while (!temp.empty()) 
        {
            RL[i].push(temp.front());
            temp.pop();
        }
    }
}

void remove_from_parent(int process_j)
{
    std::deque<int> q = PCB[PCB[process_j].parent].children;
    for (int i=0; i<q.size(); i++)
    {
        if (q[i] == process_j)
        {
            q.erase(q.begin()+i);
        }
    }
}

void destroy(int process_j) 
{
    destroy_all_children(process_j);
    remove_from_parent(process_j);
    remove_from_rl(process_j);
    release_all_r(process_j);
    pcb_index empty;
    PCB[process_j] = empty;
    //printf("%d processes destroyed", n);
}

int main(int argc, char *argv[])
{
    std::ifstream input_file("input.txt");
    std::ofstream output_file;
    output_file.open("output.txt"); 
    std::string input;
    
    //printf("> ");
    //char input[10];
    //fgets(input, 10, stdin);

    while(getline(input_file, input))
    {
        const int length = input.length();
        char* char_input = new char[length + 1];
        strcpy(char_input, input.c_str());
        char* command = strtok(char_input, " ");
        //printf("GETLINE: %s\n", input);
        //printf("COMMAND: %s\n", command);
        if (command != NULL)
        {
            if(strcmp("cr", command) == 0)
            {
                char* p = strtok(NULL, " \n\t");
                int priority = atoi(p);
                create(priority);
            }
            else if (strcmp("de", command) == 0)
            {
                char* p = strtok(NULL, " \n\t");
                int i = atoi(p);

                if (is_child(i) || i == RL[current_level].front())
                {
                    destroy(i);
                    scheduler();
                }
                else
                {
                    error_detected = true;
                }
            }
            else if (strcmp("rq", command) == 0)
            {
                char* r_input = strtok(NULL, " \n\t");
                char* k_input = strtok(NULL, " \n\t");
                int r = atoi(r_input);
                int k = atoi(k_input);
                request(r, k);
            }
            else if (strcmp("rl", command) == 0)
            {
                char* r_input = strtok(NULL, " \n\t");
                char* k_input = strtok(NULL, " \n\t");
                int r = atoi(r_input);
                int k = atoi(k_input);
                if (!process_has_resource(r) || k > (RCB[r].inventory - RCB[r].state))
                {
                    error_detected = true;
                }
                else
                {
                    release(r, k);
                }
            }
            else if (strcmp("to", command) == 0)
            {
                timeout();
            }
            else if (strcmp("in", command) == 0)
            {
                init();
            }
            else if (strcmp("\n", command) == 0)
            {
                output_file << "\n";
            }
            else
            {
                break;
            }
            if (!error_detected)
            {
                //printf("%d\n", RL[current_level].front());
                output_file << RL[current_level].front();
                output_file << " ";
            }
            else
            {
                //printf("-1\n");
                output_file << "-1 ";
                error_detected = false;
            }
        }
        else
        {
            output_file << "\n\n";
        }
        
    }
    input_file.close();
    output_file.close();
}