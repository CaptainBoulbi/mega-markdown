#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CHECK_MD_BOUND(md) (md->in_cursor < md->in_cap && md->out_cursor < md->out_cap)

typedef struct MegaData {
    char *in;
    char *out;
    int in_cap;
    int out_cap;
    int in_cursor;
    int out_cursor;
} MegaData;

void skip_begin(MegaData *md)
{
    while (CHECK_MD_BOUND(md) && (md->in[md->in_cursor] == ' ' || md->in[md->in_cursor] == '\t' || md->in[md->in_cursor] == '\n')) {
        md->out[md->out_cursor] = md->in[md->in_cursor];
        md->in_cursor++;
        md->out_cursor++;
    }
}

void skip_endline(MegaData *md)
{
    while (CHECK_MD_BOUND(md) && md->in[md->in_cursor] != '\n') {
        md->out[md->out_cursor] = md->in[md->in_cursor];
        md->in_cursor++;
        md->out_cursor++;
    }
}

int mmdeeznut(MegaData *md)
{
    assert(md->in_cap < md->out_cap && "output buffer should be bigger than the input file size.");
    while (CHECK_MD_BOUND(md)) {
        skip_begin(md);
        if (md->in[md->in_cursor] == '#') {
            int level = 1;
            while (md->in[md->in_cursor+level] == '#' && level < 6)
                level++;
            if (md->in[md->in_cursor+level] == ' ') {
                md->in_cursor += level+1;

                md->out[md->out_cursor] = '<';
                md->out[++md->out_cursor] = 'h';
                md->out[++md->out_cursor] = '0' + level;
                md->out[++md->out_cursor] = '>';
                md->out_cursor++;
                skip_endline(md);
                md->out[md->out_cursor] = '<';
                md->out[++md->out_cursor] = '/';
                md->out[++md->out_cursor] = 'h';
                md->out[++md->out_cursor] = '0' + level;
                md->out[++md->out_cursor] = '>';
                md->out_cursor++;
            }
        }
        skip_endline(md);
    }

    return md->out_cursor;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("ERR: please input only 2 parameter.\nusage: %s <input file> <output file>\n", *argv);
        exit(-69);
    }

    FILE *input_file = fopen(argv[1], "r");
    FILE *output_file = fopen(argv[2], "w");

    fseek(input_file , 0L, SEEK_END);
    int file_size = ftell(input_file);
    rewind(input_file);

    char *mmd_data = malloc(file_size);
    assert(mmd_data && "Buy more RAM!");
    fread(mmd_data, file_size, 1, input_file);

    char *html_data = malloc(file_size*2);

    MegaData md = {
        .in = mmd_data,
        .in_cap = file_size,
        .out = html_data,
        .out_cap = file_size*2,
        .in_cursor = 0,
        .out_cursor = 0,
    };
    int data_size = mmdeeznut(&md);

    printf("%s", mmd_data);
    printf("--------------------\n");
    printf("%.*s", data_size, html_data);

    fwrite(html_data, data_size, 1, output_file);

    fclose(output_file);
    fclose(input_file);
    return 0;
}
