
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>
// strdup + func time
// https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux/29402705

void iterate_user_files(
        const char *const user,
        char *path,
        bool *found_c_file,
        char *const input,
        char *const output,
        const char *const comp_out_path
) {
    // while there is a content in the dir
    DIR *dir;
    struct dirent *next;

    if (!(dir = opendir(path))) {
        return;
    }

    while (NULL != (next = readdir(dir))) {
        if (next->d_type == DT_DIR) { // if it is a folder
            // todo: if it failed
            if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0) {
                // not relevant-continue
                continue;
            }

            char next_dirname[1024] = {0};
            strncat(next_dirname, path, strlen(path));
            strcat(next_dirname, "/");
            strncat(next_dirname, next->d_name, strlen(next->d_name));

            // now path is the right path - combination of the current folder and the name of the sub-folder
            iterate_user_files(user, next_dirname, found_c_file, input, output, comp_out_path);
        } else if (next->d_type == DT_REG) {
            // reg file
            size_t nameLen = strlen((next->d_name));
            if (((next->d_name)[nameLen - 2] == '.') && ((next->d_name)[nameLen - 1] == 'c' )) {
                *found_c_file = true;
                pid_t gcc_pid = fork();
                if (0 == gcc_pid) { // inside son
                    chdir(path);
                    // compile each c file
                    char *const argv[5] = {"gcc", next->d_name, "-o", "child", NULL};
                    execvp("gcc", argv);
                } else {
                    // in parent
                    // printf("%s/%s\n", path, next->d_name);
                    int wstatus;
                    (void)waitpid(gcc_pid, &wstatus, 0);

                    // Check if compilation failed (2)
                    // according to wstatus
                    if (WEXITSTATUS(wstatus)) {
                        printf("%s COMPILATION_ERROR\n", user);
                        return;
                    }

                    // Run the output files
                    // fork -> exec "child input"
                    pid_t user_program_pid = fork();
                    if (0 == user_program_pid) {
                        // in son
                        // run the program
                        char child[256] = {0};
                        strcat(child, path);
                        strcat(child, "/child");

                        int input_fd = open(input, O_RDONLY);
                        dup2(input_fd, STDIN_FILENO);
                        close(input_fd);

                        int output_fd = open(
                                "result.txt",
                                O_WRONLY | O_TRUNC | O_CREAT,
                                S_IRWXU | S_IRWXO | S_IRWXG);
                        dup2(output_fd, STDOUT_FILENO);
                        dup2(output_fd, STDERR_FILENO);
                        close(output_fd);

                        char *const argv[2] = {child, NULL};
                        execvp(argv[0], argv);
                        printf("execvp failed\n");
                    } else {
                        time_t start_time = time(NULL);
                        // alarm(6);
                        (void)waitpid(user_program_pid, &wstatus, 0);
                        if (time(NULL) - start_time > 5) {
                            printf("%s TIMEOUT\n", user);
                            return;
                        }

                        pid_t compare_child_pid = fork();
                        if (0 == compare_child_pid) {
                            char *const argv[4] = {(char *)comp_out_path, "result.txt", (char *)output, NULL};
                            execvp(comp_out_path, argv);
                        } else {
                            (void)waitpid(compare_child_pid, &wstatus, 0);

                            if (WEXITSTATUS(wstatus) == 1) {
                                printf("%s EXCELLENT\n", user);
                            } else if (WEXITSTATUS(wstatus) == 3) {
                                printf("%s SIMILAR\n", user);
                            } else {
                                printf("%s WRONG\n", user);
                            }
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}

void iterate_user_directories(char *lines[3], const char *const comp_out_path) {
    // while there is a content in the dir
    DIR *dir;
    struct dirent *next;

    if (!(dir = opendir(lines[0]))) {
        printf("Not a valid directory\n");
        return;
    }

    while (NULL != (next = readdir(dir))) {
        if (next->d_type == DT_DIR) { // if it is a folder
            // todo: if it failed
            if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0) {
                // not relevant-continue
                continue;
            }

            char next_dirname[1024] = {0};
            strncat(next_dirname, lines[0], strlen(lines[0]));
            strcat(next_dirname, "/");
            strncat(next_dirname, next->d_name, strlen(next->d_name));

            // snprintf(next_dirname, sizeof(next_dirname), "%s/%s", path, next->d_name);
            // now path is the right path - combination of the current folder and the name of the sub-folder
            bool user_has_c_file = false;
            iterate_user_files(
                    next->d_name, next_dirname, &user_has_c_file, lines[1], lines[2],
                    comp_out_path
            );
            if (!user_has_c_file) {
                printf("%s NO_C_FILE\n", next->d_name); // (1)
            }
        }
    }

    closedir(dir);
}

//void do_nothing(int signum)
//{
//}

int main(int argc, char *argv[]) {
    const char *FilePath = argv[1];

    // open the input file
    int fd = open(FilePath, 0);
    if (fd == -1) {
        return 0;
    }

    // find the len of the file
    long length = lseek(fd, 0, SEEK_END);
    char* buffer;
    buffer = (void*) malloc(length);
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, length);

    // define array for the 3 sentences in the input file
    char *lines[3];
    int idx = 0;
    int lineIndex = 0;

    // the first line
    lines[lineIndex++] = buffer;
    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx++]='\0';
    lines[lineIndex++] = buffer + idx;

    // the second line
    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx++]='\0';
    lines[lineIndex] = buffer + idx;

    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx] = '\0';

    /* Set up the structure to specify the new action. */
    // struct sigaction new_action, old_action;
    // new_action.sa_handler = do_nothing;
    // sigemptyset(&new_action.sa_mask);
    // new_action.sa_flags = 0;
    // sigaction(SIGINT, &new_action, NULL);

    // lines[0] - first arg -> path to folder that includes sub-folders
    // lines [1] - second arg -> path to file that inside him there is input
    // lines [2] - third arg -> the right output for the src file from line 2
    char comp_out_path[256] = {0};
    getcwd(comp_out_path, sizeof(comp_out_path));
    strcat(comp_out_path, "/comp.out");
    printf("%s\n", comp_out_path);
    iterate_user_directories(lines, comp_out_path);
}

