
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

// https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux/29402705

void iterate_user_files(const char *const user, char *path, bool *found_c_file) {
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
            char next_dirname[1024];
            snprintf(next_dirname, sizeof(next_dirname), "%s/%s", path, next->d_name);
            // now path is the right path - combination of the current folder and the name of the sub-folder
            iterate_user_files(user, next_dirname, found_c_file);
        } else if (next->d_type == DT_REG) {
            // reg file
            size_t nameLen = strlen((next->d_name));
            if (((next->d_name)[nameLen - 2] == '.') && ((next->d_name)[nameLen - 1] == 'c' )) {
                *found_c_file = true;
                pid_t pid = fork();
                if (0 == pid) {
                    // inside son
                    chdir(path);
                    char *const argv[5] = {"gcc", next->d_name, "-o", "child", NULL};
                    execvp("gcc", argv);
                } else {
                    // in parent
                    printf("%s/%s\n", path, next->d_name);
                    int wstatus;
                    (void)waitpid(pid, &wstatus, 0);

                    // Check if compilation failed (2)
                    // according to wstatus
                    if (WEXITSTATUS(wstatus)) {
                        printf("%s COMPILATION_ERROR\n", user);
                    }

                    // Run
                    // fork -> exec "child input"
                }
            }
        }
    }

    closedir(dir);
}

void iterate_user_directories(char *path) {
    // while there is a content in the dir
    DIR *dir;
    struct dirent *next;

    if (!(dir = opendir(path))) {
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
            char next_dirname[1024];
            snprintf(next_dirname, sizeof(next_dirname), "%s/%s", path, next->d_name);
            // now path is the right path - combination of the current folder and the name of the sub-folder
            bool user_has_c_file = false;
            iterate_user_files(next->d_name, next_dirname, &user_has_c_file);
            if (!user_has_c_file) {
                printf("%s NO_C_FILE\n", next->d_name); // (1)
            }
        }
    }

    closedir(dir);
}


int main(int argc, char *argv[]) {
    const char *FilePath = argv[1];
    // open the input file
    int fd= open(FilePath, 0);
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
    // read char by char until \n

    // the second line
    while (buffer[idx] != '\n' && idx < length) {idx++;}
    buffer[idx++]='\0';
    lines[lineIndex] = buffer + idx;

    // now lines[0] - first arg -> path to folder that includes sub-folders
    // lines [1] - second arg -> path to file that inside him there is input
    // lines[2] - third arg -> the right output for the src file from line 2

    iterate_user_directories(lines[0]);
    // get one line - read. **no getline**
    // get another line

    /*
    char *lines[3];
    int lineindx = 0;
    line[lineindx++] = buffer;

    int idx = 0;
// read char by char until \n
    buffer[idx++]='\0';
    line[lineidx++] = buffer[idx];

// read char by char until next \n
    buffer[idx++]='\0';
    line[lineidx++] = buffer[idx];
    while (buffer[idx] != '\n' && idx < len) {idx++;}*/

}

