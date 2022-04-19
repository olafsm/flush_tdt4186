#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include"linked_list.h"

#define MAX_PATH_LENGTH 4096
#define MAX_COMMAND_SIZE 256
#define MAX_ARG_COUNT 64

#define WHITE "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define CYAN "\x1B[36m"
#define YELLOW "\x1B[33m"

char cwd[MAX_PATH_LENGTH];
char root_dir[MAX_PATH_LENGTH];

const char EOT = 0x04; //CTRL-D
const char TAB = 0x09;
const char SPACE = 0x20;
const char NEWLINE = 0x0a;

const char delimiters[5] = {EOT, SPACE, NEWLINE, TAB, '\0'};

char *args[MAX_ARG_COUNT];
char *cmd;

struct list* active_pids;

/**
 * @brief print cmd and free the allocated string
 */
void print_stat_and_free_cmd(int s, char* command) {
    if(command[strlen(command)-1] == '\n') {
        command[strlen(command)-1]='\0';
    }
    char* color = GREEN;
    if(s) {
        color = RED;
    }
    printf(YELLOW"Exit status [%s] = %s%d"WHITE"\n", command,color , s);
    free(command);
    return;
}

/**
 * @brief prints current working directory without waiting for newline
 */
void print_cwd() 
{
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
      perror("getcwd() error");
    }
    printf(CYAN "%s: " WHITE, cwd);
    return;
}

/**
 * @brief Executes command stored in args[]
 * 
 * @param n_args Number of arguments in args
 */
void exec_command(int n_args) 
{
    // Check for redirection
    int in_index = 0, out_index = 0;
    for (int i=0;i<n_args;i++) {
        if (strcmp(args[i], "<")==0) {
            if(i >= n_args-1) {
                printf("Specify path for input redirection\n");
                return;
            }
            in_index = i+1;
            args[i] = NULL;
        }
        else if(strcmp(args[i], ">")==0) {
            if(i >= n_args-1) {
                printf("Specify path for output redirection\n");
                return;
            }
            out_index = i+1;
            args[i] = NULL;
        }
    }

    // CD Command doesnt need to be a child process
    if(strcmp(args[0], "cd")==0) {
        if(args[1]==0) {
            chdir(root_dir);
            return;
        }
        chdir(args[1]);
        return;
    }

    // Check for background task
    int bg = 0;
    if(cmd[strlen(cmd)-2] == '&') {
        bg = 1;
    }

    // Execute command in child process
    int stat;
    pid_t pid = fork();
    if (pid == 0)
    {
        int stdout_fd = fileno(stdout);

        // redirect stdin & stdout before executing command
        if(in_index) 
            freopen(args[in_index], "r", stdin); 
        if(out_index)
            freopen(args[out_index], "w", stdout);

        if(strcmp(args[0], "jobs") == 0) {
            print_list(active_pids);
        } else {
            execvp(args[0], args);
        }   
        exit(0);
    }
    else if (pid == -1) 
    {
        perror("fork() error");
    }
    else if (!bg)
    {
        wait(&stat);
        if (WIFEXITED(stat)) 
        {
            print_stat_and_free_cmd(WEXITSTATUS(stat), cmd);
        }
    } else 
    {
        add_node(active_pids, pid, cmd);
    }
}

/**
 * @brief Returns exit status if process has exited, -1 if not
 * 
 * @param PID 
 * @return int 
 */
int get_zombie_status(int PID) {
    int stat;
    if (waitpid(PID, &stat, WNOHANG))
    {
        if (WIFEXITED(stat))
        {
            return WEXITSTATUS(stat);
        }
    }
    return -1;
}

/**
 * @brief Walks the linked list and removes any zombie processes
 * 
 * @param list 
 */
void remove_zombie_nodes (struct list *list) {
    struct node *n = active_pids->head->next;
    struct node *p = active_pids->head;
    while(n != NULL) {
        int stat = get_zombie_status(n->PID);
        if(stat != -1) 
        {
            print_stat_and_free_cmd(stat,n->command);
            p->next = n->next;
            free(n);
        } else {
            p = n;
        }
        n = p->next;
    }
};

/**
 * @brief Parses line from stdin into *args
 * 
 * @return int number of arguments parsed
 */
int accept_new_command() 
{
    size_t buf_size = MAX_COMMAND_SIZE;
    char* temp_cmd;
    int n;

    cmd = (char *)malloc(buf_size * sizeof(char));
    n = getline(&cmd, &buf_size, stdin);
    temp_cmd = strdup(cmd);

    if(n == -1) 
    {
        exit(0);
    }
    
    char *token;
    int arg_idx = 0;
    while (1) 
    {
        token = strsep(&temp_cmd, delimiters);
        if(token == NULL) 
        {
            return arg_idx;
        }
        if(strlen(token)>0) 
        {
            if(token[strlen(token)-1] == '&') {
                token[strlen(token)-1] = '\0';
            }
            if(strlen(token)) {
                args[arg_idx] = strdup(token);
                arg_idx++;
            }
        }
    }
    free(temp_cmd);
    return arg_idx;
}

/**
 * @brief free alloced argument strings and zero args[]
 * 
 * @param n_args
 */
void free_args(int n_args) {
    for(int i=0;i<n_args;i++) {
        free(args[i]);
    }
    memset(args, 0, sizeof(args));
}

/**
 * @brief accepts new commands from stdin and executes them
 * 
 * @return int
 */
int main() 
{
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
      perror("getcwd() error");
    }
    memcpy(root_dir, cwd, sizeof(cwd));

    active_pids = create_list();
    
    
    int n_args = 0;
    while(1) 
    {
        print_cwd();
        n_args = accept_new_command();
        if(n_args) // Segfaults if arg is empty 
        {
            exec_command(n_args);
            free_args(n_args);
        }
        remove_zombie_nodes(active_pids);
    }
    return 0;
}