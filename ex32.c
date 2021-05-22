

#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>

// Files
#define ERR_FILE "errors.txt"
#define TMP_RESULT_FILE "results.txt"
#define CHILD_BIN_NAME "child"

// Grades
#define NO_C_FILE ",0,NO_C_FILE\n"
#define NO_C_FILE_LEN (sizeof(NO_C_FILE)-1)
#define COMPILATION_ERROR ",10,COMPILATION_ERROR\n"
#define COMPILATION_ERROR_LEN (sizeof(COMPILATION_ERROR)-1)
#define TIMEOUT ",20,TIMEOUT\n"
#define TIMEOUT_LEN (sizeof(TIMEOUT)-1)
#define WRONG ",50,WRONG\n"
#define WRONG_LEN (sizeof(WRONG)-1)
#define SIMILAR ",75,SIMILAR\n"
#define SIMILAR_LEN (sizeof(SIMILAR)-1)
#define EXCELLENT ",100,EXCELLENT\n"
#define EXCELLENT_LEN (sizeof(EXCELLENT)-1)

// Checks
#define CHECK(cond, msg) do {                                           \
    if (!(cond)) {                                                      \
        write(STDOUT_FILENO, "Error in: "msg"\n", strlen(msg) + 11);    \
    }                                                                   \
} while (0);

#define CHECK_RET(cond, msg) do {                                       \
    if (!(cond)) {                                                      \
        write(STDOUT_FILENO, "Error in: "msg"\n", strlen(msg) + 11);    \
        return;                                                         \
    }                                                                   \
} while (0);

#define CHECK_RET_GOTO(cond, msg, label) do {                           \
    if (!(cond)) {                                                      \
        write(STDOUT_FILENO, "Error in: "msg"\n", strlen(msg) + 11);    \
        goto label;                                                     \
    }                                                                   \
} while (0);

#define CHECK_RET_VAL(cond, msg, val) do {              \
    if (!(cond)) {                                      \
        write(STDOUT_FILENO, msg"\n", strlen(msg) + 1); \
        return (val);                                   \
    }                                                   \
} while (0);

/**
 * Find the C file in the directory, compile and compare result.
 * @param user
 * @param path - path of the user directory
 * @param found_c_file - if found c files
 * @param input - input path
 * @param output - output path
 * @param comp_out_path - comp path
 * @param csv_fd - the fd of csv
 */
void iterate_user_files(
        const char *const user,
        char *path,
        bool *found_c_file,
        char *const input,
        char *const output,
        const char *const comp_out_path,
        int csv_fd
) {
    DIR *dir = opendir(path);
    CHECK_RET(NULL != dir, "opendir");

    struct dirent *next;

    // while there is a content in the dir
    while (NULL != (next = readdir(dir))) {
        // Handle recursive C files - Not required
        if (next->d_type == DT_REG) { // if it is a folder
            // find the length of the name
            size_t nameLen = strlen((next->d_name));
            // if it is c file
            if (((next->d_name)[nameLen - 2] == '.') && ((next->d_name)[nameLen - 1] == 'c' )) {
                *found_c_file = true;
                // fork in order to compile
                pid_t gcc_pid = fork();
                if (0 == gcc_pid) {
                    // inside son
                    int error_fd = open(
                            ERR_FILE,
                            O_APPEND | O_WRONLY,
                            S_IRWXU | S_IRWXO | S_IRWXG);
                    CHECK_RET(-1 != error_fd, "open");
                    // route the error into errors.txt
                    CHECK_RET_GOTO(-1 != dup2(error_fd, STDERR_FILENO), "dup2", out_child);
                    CHECK(-1 != close(error_fd), "close");
                    // change the path
                    CHECK_RET(-1 != chdir(path), "chdir");

                    // compile each c file
                    char *const argv[5] = {"gcc", next->d_name, "-o", CHILD_BIN_NAME, NULL};
                    CHECK_RET(-1 != execvp("gcc", argv), "execvp");

                    out_child:
                    close(error_fd);
                    return;
                } else {
                    CHECK_RET(-1 != gcc_pid, "fork");

                    // in parent
                    int wstatus;
                    CHECK_RET(-1 != waitpid(gcc_pid, &wstatus, 0), "waitpid");

                    // Check if compilation failed (2) - according to wstatus
                    if (WEXITSTATUS(wstatus)) {
                        CHECK(-1 != write(csv_fd, user, strlen(user)), "write");
                        CHECK(-1 != write(csv_fd, COMPILATION_ERROR, COMPILATION_ERROR_LEN), "write");
                        return;
                    }

                    // Run the output files
                    char child[256] = {0};
                    // combine in order to know the full path of the child
                    strcat(child, path);
                    strcat(child, "/"CHILD_BIN_NAME);
                    pid_t user_program_pid = fork();
                    if (0 == user_program_pid) {
                        // in son
                        // run the program
                        int error_fd = -1;
                        int output_fd = -1;
                        int input_fd = open(input, O_RDONLY);
                        CHECK_RET(-1 != input_fd, "open");
                        // route the stdin to the input file
                        CHECK_RET_GOTO(-1 != dup2(input_fd, STDIN_FILENO), "dup2", out_user_program1);
                        CHECK(-1 != close(input_fd), "close");

                        output_fd = open(
                                TMP_RESULT_FILE,
                                O_WRONLY | O_TRUNC | O_CREAT,
                                S_IRWXU | S_IRWXO | S_IRWXG);
                        CHECK_RET(-1 != output_fd, "open");
                        // route the stdout to the results.txt - output file
                        CHECK_RET_GOTO(-1 != dup2(output_fd, STDOUT_FILENO), "dup2", out_user_program2);
                        CHECK(-1 != close(output_fd), "close");

                        error_fd = open(
                                ERR_FILE,
                                O_APPEND | O_WRONLY,
                                S_IRWXU | S_IRWXO | S_IRWXG);
                        CHECK_RET(-1 != error_fd, "open");
                        // route the errors to the input file
                        CHECK_RET_GOTO(-1 != dup2(error_fd, STDERR_FILENO), "dup2", out_user_program3);
                        CHECK(-1 != close(error_fd), "close");

                        char *const argv[2] = {child, NULL};
                        // running the child
                        CHECK_RET(-1 != execvp(argv[0], argv), "execvp");

                        out_user_program1:
                        close(input_fd);

                        out_user_program2:
                        if (-1 != output_fd) {
                            close(output_fd);
                        }

                        out_user_program3:
                        if (-1 != error_fd) {
                            close(error_fd);
                        }
                    } else {
                        CHECK_RET(-1 != user_program_pid, "fork");
                        // use time in order to know if 5 seconds left
                        time_t start_time = time(NULL);

                        CHECK_RET(-1 != waitpid(user_program_pid, &wstatus, 0), "waitpid");
                        CHECK(-1 != write(csv_fd, user, strlen(user)), "write");
                        if (time(NULL) - start_time > 5) {
                            CHECK(-1 != write(csv_fd, TIMEOUT, TIMEOUT_LEN), "write");
                            return;
                        }

                        pid_t compare_child_pid = fork();
                        if (0 == compare_child_pid) {
                            // set the args in array
                            char *const argv[4] = {(char *)comp_out_path, TMP_RESULT_FILE, (char *)output, NULL};
                            // check if the filels are the same
                            CHECK(-1 != execvp(comp_out_path, argv), "execvp");
                        } else {
                            // wait for the children to finish
                            CHECK_RET(-1 != waitpid(compare_child_pid, &wstatus, 0), "waitpid");

                            if (WEXITSTATUS(wstatus) == 1) {
                                // if the files are the same
                                CHECK(-1 != write(csv_fd, EXCELLENT, EXCELLENT_LEN), "write");
                            } else if (WEXITSTATUS(wstatus) == 3) {
                                // if the files are similar
                                CHECK(-1 != write(csv_fd, SIMILAR, SIMILAR_LEN), "write");
                            } else {
                                // if the files are different
                                CHECK(-1 != write(csv_fd, WRONG, WRONG_LEN), "write");
                            }
                            // remove unneeded files
                            CHECK(-1 != remove(TMP_RESULT_FILE), "remove");
                            CHECK(-1 != remove(child), "remove");
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}
/**
 * Iterate user paths
 * @param lines pointer to array that includs 3 line (input file)
 * @param comp_out_path compare file path
 * @param csv_fd the fd of csv
 */
void iterate_user_directories(char *lines[3], const char *const comp_out_path, int csv_fd) {
    // while there is a content in the dir
    DIR *dir = opendir(lines[0]);
    CHECK_RET(NULL != dir, "opendir");

    struct dirent *next;
    while (NULL != (next = readdir(dir))) {
        if (next->d_type == DT_DIR) {
            // if it is a folder
            if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0) {
                // not relevant-continue
                continue;
            }

            // now path is the right path - combination of the current folder and the name of the sub-folder
            char next_dirname[1024] = {0};
            // combine in order to save the whole path
            strncat(next_dirname, lines[0], strlen(lines[0]));
            strcat(next_dirname, "/");
            strncat(next_dirname, next->d_name, strlen(next->d_name));

            bool user_has_c_file = false;
            iterate_user_files(
                    next->d_name, next_dirname, &user_has_c_file, lines[1], lines[2],
                    comp_out_path, csv_fd
            );
            if (!user_has_c_file) {
                CHECK(-1 != write(csv_fd, next->d_name, strlen(next->d_name)), "write");
                CHECK(-1 != write(csv_fd, NO_C_FILE, NO_C_FILE_LEN), "write");
            }
        }
    }

    CHECK(-1 != closedir(dir), "closedir");
}


/**
 *
 * @param argc
 * @param argv file that inside:
 * line 1 - path to a folder that include sub-folders,
 * line 2- path to file that include input
 * line 3 - path to a file that include the correct output for the input file from line 2
 * @return
 */
int main(int argc, char *argv[]) {
    const char *FilePath = argv[1];

    int res = -1;
    char *buffer = NULL;

    // open the input file
    int fd = open(FilePath, 0);
    CHECK_RET_VAL(-1 != fd, "Error in: open", -1);

    // find the len of the file
    long length = lseek(fd, 0, SEEK_END);
    buffer = (void *)malloc(length);
    CHECK_RET_GOTO(NULL != buffer, "Error in: malloc", out);
    // set to start
    lseek(fd, 0, SEEK_SET);
    CHECK_RET_GOTO(-1 != read(fd, buffer, length), "Error in: read", out);

    // define array for the 3 sentences in the input file
    char *lines[3];
    int idx = 0;
    int lineIndex = 0;

    // the first line
    lines[lineIndex++] = buffer;
    while (buffer[idx] != '\n' && idx < length) {idx++;}

    // the second line
    buffer[idx++]='\0';
    DIR *p = opendir(lines[0]);
    CHECK_RET_GOTO(NULL != p, "Not a valid directory", out);
    CHECK_RET_GOTO(-1 != closedir(p), "closedir", out);

    lines[lineIndex++] = buffer + idx;

    // the third line
    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx++]='\0';
    int tmpfd = open(lines[1], 0);
    CHECK_RET_GOTO(-1 != tmpfd, "Input file not exist", out);
    CHECK_RET_GOTO(-1 != close(tmpfd), "close", out);

    lines[lineIndex] = buffer + idx;

    // nullbyte at the end instead of newline
    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx] = '\0';
    tmpfd = open(lines[1], 0);
    CHECK_RET_GOTO(-1 != tmpfd, "Output file not exist", out);
    CHECK_RET_GOTO(-1 != close(tmpfd), "close", out);

    // Create error file
    int err_fd = open(ERR_FILE, O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO | S_IRWXG);
    CHECK_RET_GOTO(-1 != close(err_fd), "close", out); // Will be opened when needed

    // lines [0] - first arg -> path to folder that includes sub-folders
    // lines [1] - second arg -> path to file that inside him there is input
    // lines [2] - third arg -> the right output for the src file from line 2
    char comp_out_path[256] = {0};
    CHECK_RET_GOTO(NULL != getcwd(comp_out_path, sizeof(comp_out_path)), "getcwd", out);
    // combine the whole path
    strcat(comp_out_path, "/comp.out");

    int csv_fd = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXO | S_IRWXG);
    iterate_user_directories(lines, comp_out_path, csv_fd);
    CHECK_RET_GOTO(-1 != close(csv_fd), "close", out);

    res = 0;

    out:
    close(fd);

    if (buffer != NULL) {
        free(buffer);
    }
    return res;
}
