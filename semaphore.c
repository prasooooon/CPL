#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>

static sem_t daemon_shutdown;

static void exit_handler(int sig) {
    sem_post(&daemon_shutdown);
}

static int set_signal_handler(int sig, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handler;
    sigemptyset(&(sa.sa_mask));
    sa.sa_flags = 0;
    if(sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    return 0;
}

// DEFINE THESE FOR YOUR PROGRAM:
int do_daemon();
void do_exit();
//

int main() {
    assert(0 == sem_init(&daemon_shutdown, 0, 0));
    if(!do_daemon()) {
        do_exit();
        return 1;
    }
    if(
            set_signal_handler(SIGHUP, exit_handler) != 0 ||
            set_signal_handler(SIGINT, exit_handler) != 0 ||
            set_signal_handler(SIGTERM, exit_handler) != 0 ||
            set_signal_handler(SIGPIPE, SIG_IGN) != 0) {
        do_exit();
        return 2;
    }
    sem_wait(&daemon_shutdown);
    do_exit();
    return 0;
}