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

char *args[MAX_COMMAND_SIZE][MAX_ARG_COUNT];
char cmd[MAX_COMMAND_SIZE];
void print_cwd() {
    printf("%s: ", cwd);
    fflush(stdout);
    return;
}
void accept_new_command() {
    char command[MAX_COMMAND_SIZE];
    int cur_arg = 0;
    int len = 0;
    fflush(stdout);
    memset(command,0,sizeof(command));
    while(read(0,buf,sizeof(buf))>0){
        switch (buf[0])
        {
        case TAB:
        case SPACE:
            *args[cur_arg] = malloc(sizeof(command));
            memcpy(*args[cur_arg], command, sizeof(command));
            memset(command,0,sizeof(command));
            len = 0;
            cur_arg +=1;
            break;
        case EOT: // CTRL+D
            _exit(0);
            break;
        case NEWLINE: // End of command
            *args[cur_arg] = malloc(sizeof(command));
            memcpy(*args[cur_arg], command, sizeof(command));
            
            int stat;
            pid_t pid;

            if (fork() == 0) {
                execv(*args[0], args[1]);
                exit(0);
            }
            else {
                waitpid(0,&stat, 0);
            }
            if (WIFEXITED(stat)) {
                printf("Exit status [%s", *args[0]);
                for(int i=1;i<=cur_arg;i++) {
                    if(*args[i] != NULL) {
                        printf(" %s", *args[i]);
                        free(*args[i]);
                    }
                }
                printf("] = %d\n", WEXITSTATUS(stat));
            }
            cur_arg = 0;
            len = 0;
            buf[0] = ' ';
            print_cwd();
        default:
            command[len] = buf[0];
            len+=1;
            break;
        }
    }
    return;
}
int main() {
    if (getcwd(cwd, sizeof(cwd)) == NULL)
      perror("getcwd() error");
    memcpy(base_cwd, cwd, sizeof(cwd));
    print_cwd();
    
    accept_new_command();
    return 0;
}