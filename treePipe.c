#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdarg.h> // For variable arguments in print_message()

// Function to print messages to different outputs
void print_message(int output_type, void *output_dest, const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (output_type == 0) { 
        // Standard output (printf)
        vprintf(format, args);
    } 
    else if (output_type == 1) { 
        // Standard error (fprintf to stderr)
        vfprintf(stderr, format, args);
    } 
    else if (output_type == 2) { 
        // File descriptor output (dprintf)
        int fd = *((int *)output_dest);
        vdprintf(fd, format, args);
    } 
    else if (output_type == 3) { 
        // String buffer (snprintf)
        char *buffer = (char *)output_dest;
        vsnprintf(buffer, 256, format, args);
    }

    va_end(args);
}

// Function to print depth-based messages
void print_depth(int depth, const char *message) {
    for (int i = 0; i < depth; i++) {
        print_message(1, NULL, "---");
    }
    print_message(1, NULL, "> %s\n", message);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        print_message(0, NULL, "Usage: ./treePipe <current depth> <max depth> <left-right>\n");
        return 1;
    }

    int curDepth = atoi(argv[1]);
    int maxDepth = atoi(argv[2]);
    int lr = atoi(argv[3]);

    if (curDepth < 0 || maxDepth < 0 || (lr != 0 && lr != 1)) {
        print_message(1, NULL, "Invalid arguments\n");
        return 1;
    }

    char output_msg[256];
    print_message(3, output_msg, "current depth: %d, lr: %d", curDepth, lr);
    print_depth(curDepth, output_msg);

    int num1;
    if (curDepth == 0) {
        print_message(1, NULL, "Please enter num1 for the root: ");
        scanf("%d", &num1);
    } else {
        scanf("%d", &num1);
    }

    print_message(3, output_msg, "my num1 is: %d", num1);
    print_depth(curDepth, output_msg);

    int num2;
    if (curDepth == maxDepth) {
        num2 = 1; // Leaf node default value
    } 
    else {
        int pipe_parent_to_leftchild[2], pipe_leftchild_to_parent[2];
        if (pipe(pipe_parent_to_leftchild) == -1 || pipe(pipe_leftchild_to_parent) == -1) {
            perror("pipe left failed");
            exit(1);
        }

        int pid_left = fork();
        if (pid_left < 0) {
            perror("fork left failed");
            exit(1);
        }

        if (pid_left == 0) { // Left child
            close(pipe_parent_to_leftchild[1]);
            close(pipe_leftchild_to_parent[0]);
            dup2(pipe_parent_to_leftchild[0], STDIN_FILENO);
            dup2(pipe_leftchild_to_parent[1], STDOUT_FILENO);
            close(pipe_parent_to_leftchild[0]);
            close(pipe_leftchild_to_parent[1]);

            char curDepth_str[11], maxDepth_str[11];
            sprintf(curDepth_str, "%d", curDepth + 1);
            sprintf(maxDepth_str, "%d", maxDepth);
            char *args[] = {"./treePipe", curDepth_str, maxDepth_str, "0", NULL};
            execvp(args[0], args);
            perror("execvp left failed");
            exit(1);
        } 
        else { // Parent
            close(pipe_parent_to_leftchild[0]);
            close(pipe_leftchild_to_parent[1]);
            print_message(2, &pipe_parent_to_leftchild[1], "%d\n", num1);
            close(pipe_parent_to_leftchild[1]);

            wait(NULL);
            char buffer[11];
            read(pipe_leftchild_to_parent[0], buffer, 10);
            num2 = atoi(buffer);
            close(pipe_leftchild_to_parent[0]);
        }
    }

    int pipe_parent_to_child[2], pipe_child_to_parent[2];
    if (pipe(pipe_parent_to_child) == -1 || pipe(pipe_child_to_parent) == -1) {
        perror("pipe self failed");
        exit(1);
    }

    int pid_self = fork();
    if (pid_self < 0) {
        perror("fork self failed");
        exit(1);
    }

    int result;
    if (pid_self == 0) { // Child executes left/right
        close(pipe_parent_to_child[1]);
        close(pipe_child_to_parent[0]);
        dup2(pipe_parent_to_child[0], STDIN_FILENO);
        dup2(pipe_child_to_parent[1], STDOUT_FILENO);
        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);

        char *prog = (lr == 0) ? "./left" : "./right";
        char *args[] = {prog, NULL};
        execvp(args[0], args);
        perror("execvp self failed");
        exit(1);
    } 
    else { // Parent
        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);
        print_message(2, &pipe_parent_to_child[1], "%d\n%d\n", num1, num2);
        close(pipe_parent_to_child[1]);

        wait(NULL);
        char buffer[11];
        read(pipe_child_to_parent[0], buffer, 10);
        result = atoi(buffer);
        close(pipe_child_to_parent[0]);

        if (curDepth == maxDepth) {
            print_message(3, output_msg, "my result is: %d", result);
            print_depth(curDepth, output_msg);
        }
    }

    int right_res = result;
    if (curDepth < maxDepth) {
        print_message(3, output_msg, "current depth: %d, lr: %d, my num1: %d, my num2: %d", curDepth, lr, num1, num2);
        print_depth(curDepth, output_msg);
        print_message(3, output_msg, "my result is: %d", result);
        print_depth(curDepth, output_msg);

        int pipe_parent_to_right_child[2], pipe_right_child_to_parent[2];
        if (pipe(pipe_parent_to_right_child) == -1 || pipe(pipe_right_child_to_parent) == -1) {
            perror("pipe right failed");
            exit(1);
        }

        int pid_right = fork();
        if (pid_right < 0) {
            perror("fork right failed");
            exit(1);
        }

        if (pid_right == 0) { // Right child
            close(pipe_parent_to_right_child[1]);
            close(pipe_right_child_to_parent[0]);
            dup2(pipe_parent_to_right_child[0], STDIN_FILENO);
            dup2(pipe_right_child_to_parent[1], STDOUT_FILENO);
            close(pipe_parent_to_right_child[0]);
            close(pipe_right_child_to_parent[1]);

            char curDepth_str[11], maxDepth_str[11];
            sprintf(curDepth_str, "%d", curDepth + 1);
            sprintf(maxDepth_str, "%d", maxDepth);
            char *args[] = {"./treePipe", curDepth_str, maxDepth_str, "1", NULL};
            execvp(args[0], args);
            perror("execvp right failed");
            exit(1);
        } 
        else { // Parent
            close(pipe_parent_to_right_child[0]);
            close(pipe_right_child_to_parent[1]);
            print_message(2, &pipe_parent_to_right_child[1], "%d\n", result);
            close(pipe_parent_to_right_child[1]);

            wait(NULL);
            char buffer[11];
            read(pipe_right_child_to_parent[0], buffer, 10);
            right_res = atoi(buffer);
            close(pipe_right_child_to_parent[0]);
        }
    }

    if (curDepth == 0) {
        print_message(1, NULL, "The final result is: %d\n", right_res);
    } 
    else {
        print_message(0, NULL, "%d\n", right_res);
    }

    return 0;
}
