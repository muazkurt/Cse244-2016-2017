#include "Listdir.h"
int logfile = 0;
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "%s \"string\" <filename>\n", argv[0]);
        return -1;
    }
    listDir(argv[1], argv[2]);
}