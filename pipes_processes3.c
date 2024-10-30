/* pipes_processes3.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search-term>\n", argv[0]);
        exit(1);
    }

    char *search_term = argv[1];  // Argument to pass to grep
    int pipe1[2];  // Pipe between cat and grep
    int pipe2[2];  // Pipe between grep and sort

    // Create the first pipe
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(1);
    }

    // Fork the first child process (P1 - Parent, executes "cat scores")
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // In the first child process (executes "cat scores")
        dup2(pipe1[1], STDOUT_FILENO);  // Redirect stdout to pipe1 write end
        close(pipe1[0]);  // Close unused read end of pipe1
        close(pipe1[1]);

        execlp("cat", "cat", "scores", (char *)NULL);  // Execute "cat scores"
        perror("execlp cat");  // If execlp fails
        exit(1);
    }

    // Create the second pipe
    if (pipe(pipe2) == -1) {
        perror("pipe2");
        exit(1);
    }

    // Fork the second child process (P2 - Child, executes "grep <argument>")
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        // In the second child process (executes "grep <argument>")
        dup2(pipe1[0], STDIN_FILENO);   // Redirect stdin to pipe1 read end
        dup2(pipe2[1], STDOUT_FILENO);  // Redirect stdout to pipe2 write end
        close(pipe1[1]);  // Close unused write end of pipe1
        close(pipe1[0]);
        close(pipe2[0]);  // Close unused read end of pipe2
        close(pipe2[1]);

        execlp("grep", "grep", search_term, (char *)NULL);  // Execute "grep <argument>"
        perror("execlp grep");  // If execlp fails
        exit(1);
    }

    // Close the first pipe as it's no longer needed in the parent
    close(pipe1[0]);
    close(pipe1[1]);

    // Fork the third child process (P3 - executes "sort")
    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid3 == 0) {
        // In the third child process (executes "sort")
        dup2(pipe2[0], STDIN_FILENO);  // Redirect stdin to pipe2 read end
        close(pipe2[1]);  // Close unused write end of pipe2
        close(pipe2[0]);

        execlp("sort", "sort", (char *)NULL);  // Execute "sort"
        perror("execlp sort");  // If execlp fails
        exit(1);
    }

    // Close the second pipe as it's no longer needed in the parent
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for all child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}
