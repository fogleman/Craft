#include "parser.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "item.h"

typedef struct {
    #define FILEPATH_LEN 256
    char filepath[256];
    FILE *file;
} Parser;

void init(Parser *m, const char *filepath);
bool open_file(Parser *m);
bool read_all(Parser *m);
void parse_line(Parser *m, const char *line);
void close_file(Parser *m);


void parser_parse_all() {
    static const char filename[] = "def.txt";

    Parser m;
    init(&m, filename);

    if(!open_file(&m)) exit(1);
    if(!read_all(&m)) exit(1);
    close_file(&m);
}

void init(Parser *m, const char *filepath) {
    //Ensure null termination. strncpy will not null terminate
    // if the length of src is n! Security!
    strncpy(m->filepath, filepath, FILEPATH_LEN - 1);
    m->filepath[FILEPATH_LEN - 1] = '\0';

    m->file = NULL;
}

bool open_file(Parser *m) {
    FILE *f = fopen(m->filepath, "r");

    if(f) {
        m->file = f;
        return true;
    } else {
        perror(m->filepath);
        return false;
    }
}

bool read_all(Parser *m) {
    #define READ_ALL_BUF_LEN 256

    bool read_at_least_one = false;

    while(1) {
        char buf[READ_ALL_BUF_LEN];
        if(!fgets(buf, READ_ALL_BUF_LEN, m->file)) {
            if(!read_at_least_one) fprintf(stderr, "%s contains no definitions\n", m->filepath);
            return read_at_least_one;
        }
        read_at_least_one = true;

        printf("read: %s", buf);

        parse_line(m, buf);
    }

    return true;
}

void parse_line(Parser *m, const char *line) {
    ItemSpec s = {};

    sscanf(line, ITEMSPEC_NAME_FORMAT " %d", s.name, &s.obstacle);

    printf("spec: name (%s), obstacle(%d)\n\n", s.name, s.obstacle);
}

void close_file(Parser *m) {
    fclose(m->file);
}
