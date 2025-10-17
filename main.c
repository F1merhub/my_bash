#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "bash.h"

#define MAX_CMD_LENGTH 255
#define STDIN_FD 0
#define STDOUT_FD 1
#define PIPE_READ 0
#define PIPE_WRITE 1
#define FAKE_FD -1

int main(int argc, char* argv[]) {
    char input_line[MAX_CMD_LENGTH];
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        fgets(input_line, MAX_CMD_LENGTH, stdin);
        if (input_line[0] == 'q' && (input_line[1] == '\n' || input_line[1] == '\0'))
            break;

        char* input_file = NULL;
        char* output_file = NULL;
        char* command_str = input_line;
        
                char* lt_pos = strchr(input_line, '<');
        if (lt_pos) {
            *lt_pos = '\0';
            input_file = lt_pos + 1;
            
            while (*input_file == ' ') {
                input_file++;
            }
            
            char* filename_end = input_file;
            while (*filename_end && *filename_end != ' ' && *filename_end != '\n') {
                filename_end++;
            }
            *filename_end = '\0';
        }
        
        char* gt_pos = strchr(input_line, '>');
        if (gt_pos) {
            *gt_pos = '\0';
            output_file = gt_pos + 1;
            
            while (*output_file == ' ') {
                output_file++;
            }
            
            char* filename_end = output_file;
            while (*filename_end && *filename_end != ' ' && *filename_end != '\n') {
                filename_end++;
            }
            *filename_end = '\0';
        }

        char* save_position;
        char* single_command = strtok_r(command_str, "|\n", &save_position);

        char*** commands_list = NULL;
        int commands_count = 0;
        
        while (single_command) {
            commands_count++;
            commands_list = (char***)realloc(commands_list, sizeof(char**) * commands_count);
            commands_list[commands_count - 1] = parse_arguments(single_command);
            single_command = strtok_r(NULL, "|\n", &save_position);
        }
        
        if (commands_count > 0) {
            run_commands(commands_list, commands_count, input_file, output_file);
        }

        for (int i = 0; i < commands_count; i++) {
            for (int j = 0; commands_list[i][j] != NULL; j++) {
                free(commands_list[i][j]);
            }
            free(commands_list[i]);
        }
        free(commands_list);
    }

    return 0;
}

char** parse_arguments(char* command) {
    char* save_position;
    char* argument = strtok_r(command, " \n", &save_position);
    
    char** arguments_array = NULL;
    int arguments_count = 0;
    
    while (argument) {
        arguments_count++;
        arguments_array = (char**)realloc(arguments_array, sizeof(char*) * arguments_count);
        arguments_array[arguments_count - 1] = strdup(argument);
        argument = strtok_r(NULL, " \n", &save_position);
    }
    
    arguments_array = (char**)realloc(arguments_array, sizeof(char*) * (arguments_count + 1));
    arguments_array[arguments_count] = NULL;

    return arguments_array;
}

void run_commands(char*** commands_array, int command_count, char* input_file, char* output_file) {
    int pipefd_arr[command_count + 1][2];
    
    for (int i = 1; i < command_count; i++) {
        open_pipe(pipefd_arr[i]);
    }
    
    if (input_file != NULL) {
        pipefd_arr[0][PIPE_READ] = safe_open(input_file, O_RDONLY, 0);
    } else {
        pipefd_arr[0][PIPE_READ] = STDIN_FD;
    }
    pipefd_arr[0][PIPE_WRITE] = FAKE_FD;

    if (output_file != NULL) {
        pipefd_arr[command_count][PIPE_WRITE] = safe_open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        pipefd_arr[command_count][PIPE_WRITE] = STDOUT_FD;
    }
    pipefd_arr[command_count][PIPE_READ] = FAKE_FD;

    for (int i = 0; i < command_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error in fork");
            exit(1);
        } else if (pid == 0) {
            if (pipefd_arr[i][PIPE_READ] != STDIN_FD) {
                safe_close(STDIN_FD);             
                if (dup(pipefd_arr[i][PIPE_READ]) == -1) {
                    perror("error in dup");
                    exit(1);
                }
            }

            if (pipefd_arr[i + 1][PIPE_WRITE] != STDOUT_FD) {
                safe_close(STDOUT_FD);
                if (dup(pipefd_arr[i + 1][PIPE_WRITE]) == -1) {
                    perror("error in dup");
                    exit(1);
                }
            }

            for (int j = 0; j <= command_count; j++) {
                if (j != i && pipefd_arr[j][PIPE_READ] != FAKE_FD && pipefd_arr[j][PIPE_READ] != STDIN_FD) {
                    safe_close(pipefd_arr[j][PIPE_READ]);
                }
                if (j != i + 1 && pipefd_arr[j][PIPE_WRITE] != FAKE_FD && pipefd_arr[j][PIPE_WRITE] != STDOUT_FD) {
                    safe_close(pipefd_arr[j][PIPE_WRITE]);
                }
            }
          
            execvp(commands_array[i][0], commands_array[i]);
            perror("error in execvp");
            exit(1);
        }
    }

    for (int j = 0; j <= command_count; j++) {
        if (pipefd_arr[j][PIPE_READ] != FAKE_FD && pipefd_arr[j][PIPE_READ] != STDIN_FD) {
            safe_close(pipefd_arr[j][PIPE_READ]);
        }
        if (pipefd_arr[j][PIPE_WRITE] != FAKE_FD && pipefd_arr[j][PIPE_WRITE] != STDOUT_FD) {
            safe_close(pipefd_arr[j][PIPE_WRITE]);
        }
    }

    for (int i = 0; i < command_count; i++) {
        wait(NULL);
    }
}