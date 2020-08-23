//
// Created by 杜悦 on 2020/7/14.
//

#ifndef ASSIGNMENT1_MYFILE_H
#define ASSIGNMENT1_MYFILE_H
#include <iostream>
#include <algorithm>

using namespace std;

class ReadFile {
    FILE *file;

public:
    ReadFile(char *fileName) {
        FILE *fp = fopen((char *) fileName, "r");
        if (!fp) {
            printf("can't open file: %s\n", fileName);
            exit(-1);
        }
    }

    // return false if no data in file
    bool nextLine(char *line, int length) {
        if (feof(file)) {
            return false;
        }
        line[0] = '\0';
        fgets(line, length, file);
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        return true;
    }

    void close() {
        fclose(file);
    }
};

#endif //ASSIGNMENT1_MYFILE_H
