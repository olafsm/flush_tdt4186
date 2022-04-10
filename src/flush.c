#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

#define PATH_MAX 4096
#define MAX_COMMAND_SIZE 64
#define MAX_ARG_COUNT 64

char buf[1];
char cwd[PATH_MAX];
char base_cwd[PATH_MAX];

const char EOT = 0x04;
const char TAB = 0x09;
const char SPACE = 0x20;
const char NEWLINE = '\n';

const char delimiters[4] = {SPACE, TAB, NEWLINE};

char *args[MAX_COMMAND_SIZE];

/**
 * @brief prints current working directory
 */
void print_cwd() {
    printf("%s: ", cwd);
    fflush(stdout);
    return;
}

/**
 * @brief Executes command stored in args[]
 * 
 * @param n_args Number of arguments in args
 */
void exec_command(int n_args) {
    int stat;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        exit(0);
    }
    else {
        wait(NULL);
    }
    if (WIFEXITED(stat)) {
        printf("Exit status [%s", args[0]);
        for(int i=1;i<=n_args;i++) {
            if(args[i] != NULL) {
                printf(" %s", args[i]);
                memset(args[i],0,sizeof(*args[i]));
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
int accept_new_command() {
    char *cmd_buf;
    size_t buf_size = MAX_COMMAND_SIZE;
    cmd_buf = (char *)malloc(buf_size * sizeof(char));
    
    int n = getline(&cmd_buf, &buf_size, stdin);
    
    if(n == -1) {
        exit(0);
    }
    
    char *token;
    int arg_idx = 0;
    while (1) {
        token = strsep(&cmd_buf, delimiters);
        if(token == NULL) {
            return arg_idx;
        }
        if(strlen(token)>0) {
            args[arg_idx] = token;
            arg_idx++;
        }
    }
    return arg_idx;
}

int main() {
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("getcwd() error");
    }
    memcpy(base_cwd, cwd, sizeof(cwd));
    int n_args = 0;
    while(1) {
        print_cwd();
        n_args = accept_new_command();
        exec_command(n_args);
    }
    return 0;
}