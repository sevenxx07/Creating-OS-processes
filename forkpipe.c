#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

volatile sig_atomic_t stop_signal = 1;

void signal_handler(int stop){
    if(stop == SIGTERM){
        stop_signal = 0;
    }
}

int main(int argc, char *argv[])
{
    int pfd[2];
    int p1, p2;
    int ret;

    //creating pipe
    ret = pipe(pfd);
    if (ret < 0 ) return 2;

    //GEN
    p1 = fork();
    if(p1 == 0){
        //duplicating writing to pipe to stdout and closing fd i dont need
        ret = close(pfd[0]);
        if (ret < 0 ) return 2;
        ret = dup2(pfd[1], STDOUT);
        if (ret < 0 ) return 2;
        ret = close(pfd[1]);
        if (ret < 0 ) return 2;

        //signal handler
        struct sigaction sigterm_action;
        memset(&sigterm_action, 0, sizeof(sigterm_action));
        sigterm_action.sa_handler = &signal_handler;
        sigterm_action.sa_flags = 0;

        if (sigaction(SIGTERM, &sigterm_action, NULL) != 0){
            perror("sigaction SIGTERM");
            return 2;
        }

        while(stop_signal){
            printf("%d %d\n", rand() % 4096, rand() % 4096);
            fflush(stdout);
            sleep(1);
        }

        fprintf(stderr, "GEN TERMINATED\n");
        return 0;
    }else if (p1 < 0){
      return 2;
    }

    //NSDS
    p2 = fork();
    if(p2 == 0){
        //duplicating reading from pipe to stdin and closing fd i dont need
        ret = close(pfd[1]);
        if (ret < 0 ) return 2;
        ret = dup2(pfd[0], STDIN);
        if (ret < 0 ) return 2;
        ret = close(pfd[0]);
        if (ret < 0 ) return 2;

        ret = execl("nsd", "nsd", NULL);
        if (ret < 0 ) return 2;
        return 0;
    }else if (p2 < 0){
      return 2;
    }

    //PARENT
    //close pipe 
    ret = close(pfd[0]);
    if (ret < 0 ) return 2;
    ret = close(pfd[1]);
    if (ret < 0 ) return 2;

    //waiting for 5 s and kill p1
    sleep(5);
    ret = kill(p1, SIGTERM);

    int status;
    int ex_status1 = 0, ex_status2 = 0; 

    //waiting for ending of GEN and NSD 
    ret = waitpid(p1, &status, 0);
    if (ret < 0 ) return 2;
    if(WIFEXITED(status)){ //WIFEXITED(status) true when the child process ended normally
        ex_status1 = WEXITSTATUS(status);
    }

    ret = waitpid(p2, &status, 0);
    if (ret < 0 ) return 2;
    if(WIFEXITED(status)){
        ex_status2 = WEXITSTATUS(status);
    }

    if(ex_status1 || ex_status2){
        printf("ERROR\n");
        return 1;
    }else{
        printf("OK\n");
        return 0;
    }
}
