#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_PATH_LENGTH 4096
#define MAX_COMMAND_SIZE 256
#define MAX_ARG_COUNT 64

char cwd[MAX_PATH_LENGTH];
char root_dir[MAX_PATH_LENGTH];

const char EOT = 0x04; //CTRL-D
const char TAB = 0x09;
const char SPACE = 0x20;
const char NEWLINE = 0x0a;

const char delimiters[4] = {EOT, SPACE, NEWLINE};

char *args[MAX_ARG_COUNT];

/**
 * @brief prints current working directory without waiting for newline
 */
void print_cwd() 
{
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
      perror("getcwd() error");
    }
    printf("%s: ", cwd);
    fflush(stdout);
    return;
}

/**
 * @brief Executes command stored in args[]
 * 
 * @param n_args Number of arguments in args
 */
void exec_command(int n_args) 
{
    int in_index = 0, out_index = 0;
    for (int i=0;i<n_args;i++) {
        if (strcmp(args[i], "<")==0) {
            if(i >= n_args-1) {
                printf("Specify path for input redirection");
            }
            in_index = i+1;
            args[i] = NULL;
        }
        else if(strcmp(args[i], ">")==0) {
            if(i >= n_args-1) {
                printf("Specify path for output redirection");
            }
            out_index = i+1;
            args[i] = NULL;
        }
    }
    if(strcmp(args[0], "cd")==0) {
        if(args[1]==0) {
            chdir(root_dir);
            return;
        }
        chdir(args[1]);
        return;
    }
    int stat;
    pid_t pid = fork();
    if (pid == 0)
    {
        int stdout_fd = fileno(stdout);

        // redirect stdin & stdout before executing command
        if(in_index!=0) {
            freopen(args[in_index], "r", stdin); 
        }
        if(out_index!=0) {
            freopen(args[out_index], "a+", stdout);
        }
        execvp(args[0], args);
        exit(0);
    }
    else 
    {
        wait(NULL);
    }
    if (WIFEXITED(stat)) 
    {
        printf("Exit status [%s", args[0]);
        for(int i=1;i<=n_args;i++) 
        {
            if(args[i] != NULL) 
            {
                printf(" %s", args[i]);
            }
        }
        printf("] = %d\n", WEXITSTATUS(stat));
    }
}

/**
 * @brief Parses line from stdin into *args
 * 
 * @return int number of arguments parsed
 */
int accept_new_command() 
{
    char *cmd_buf;
    size_t buf_size = MAX_COMMAND_SIZE;
    cmd_buf = (char *)malloc(buf_size * sizeof(char));
    
    int n = getline(&cmd_buf, &buf_size, stdin);
    
    if(n == -1) 
    {
        exit(0);
    }
    
    char *token;
    int arg_idx = 0;
    while (1) 
    {
        token = strsep(&cmd_buf, delimiters);
        if(token == NULL) 
        {
            return arg_idx;
        }
        if(strlen(token)>0) 
        {
            args[arg_idx] = strdup(token);
            arg_idx++;
        }
    }
    free(cmd_buf);
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
    print_cwd();
    memcpy(root_dir, cwd, sizeof(cwd));
    
    int n_args = 0;
    while(1) 
    {
        n_args = accept_new_command();
        exec_command(n_args);
        free_args(n_args);
        print_cwd();
    }
    return 0;
}