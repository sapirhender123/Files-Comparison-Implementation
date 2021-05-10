#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <ctype.h>


int main(int argc, char *argv[]) {
    if (argc < 3) {
        return -1;
    }

    const char *firstFilePath = argv[1];
    const char *secondFilePath = argv[2];

    // todo: empty file
    int firstFD= open(firstFilePath, 0);
    // If the file doesn't exist
    if (firstFD == -1) {
        return 0;
    }
    long lengthFirstFile = lseek(firstFD, 0, SEEK_END);
    char* bufferFirstFile;
    bufferFirstFile = (void*) malloc(lengthFirstFile);
    lseek(firstFD, 0, SEEK_SET);
    read(firstFD, bufferFirstFile, lengthFirstFile);
    close(firstFD);

    int secondFD= open(secondFilePath, 0);
    // If the file doesn't exist
    if (secondFD == -1) {
        return 0;
    }
    long lengthSecondFile = lseek(secondFD, 0, SEEK_END);
    char* bufferSecondFile;
    bufferSecondFile = (void*) malloc(lengthSecondFile);
    lseek(secondFD, 0, SEEK_SET);
    read(secondFD, bufferSecondFile, lengthSecondFile);
    close(secondFD);

    bool equal = lengthFirstFile == lengthSecondFile;

    int ind1 = 0, ind2 = 0;
    do { // read one char each time
        char c1 = bufferFirstFile[ind1++];
        char c2 = bufferSecondFile[ind2++];
        if (c1 != c2) {
            equal = false;
        }

        if (!equal) {
            // Case whitespaces
            while ((c1 == '\t') || (c1 == '\n') || (c1 == ' ')) {
                c1 = bufferFirstFile[ind1++];
                if (ind1 == lengthFirstFile + 1) {
                    break;
                }
            }
            while ((c2 == '\t') || (c2 == '\n') || (c2 == ' ')) {
                c2 = bufferSecondFile[ind2++];
                if (ind2 == lengthSecondFile + 1) {
                    break;
                }
            }
        } else { // if there is option that the files are equal
            continue;
        }

        // convert to lowercase
        c1 = (char)tolower(c1);
        c2 = (char)tolower(c2);
        // c1 and c2 are not whitespaces
        if (c1 != c2){
            return 2;
        }

        // If we arrived to the end of the file
    } while (ind1 != lengthFirstFile + 1 && ind2 != lengthSecondFile + 1);

    if (equal) {
        return 1;
    }

    return 3;
}
