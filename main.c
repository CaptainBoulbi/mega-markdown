#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define CHECK_MD_BOUND(md) (md->in_cursor < md->in_cap && md->out_cursor < md->out_cap)

typedef struct MegaData {
    char *in;
    char *out;
    int in_cap;
    int out_cap;
    int in_cursor;
    int out_cursor;
    int list_level;
    int list_item_added;
} MegaData;

#define md_append(md, str) md_append_imp(md, str, sizeof(str)-1)

void md_append_imp(MegaData *md, const char *str, const int len)
{
    for (int i=0; i<len; i++) {
        md->out[++md->out_cursor] = str[i];
    }
}

int skip_begin(MegaData *md)
{
    int before = md->in_cursor;
    while (CHECK_MD_BOUND(md) && (md->in[md->in_cursor] == ' ' || md->in[md->in_cursor] == '\t' || md->in[md->in_cursor] == '\n')) {
        md->out[md->out_cursor] = md->in[md->in_cursor];
        md->in_cursor++;
        md->out_cursor++;
        // NOTE: c'est tres moche, la balise ne devrait pas etre ajouter ici
        //       j'ai mis ça la parce que skip_begin() saute meme les lignes vides et j'ai pas le temps de tout refacto avant ce soir
        //       donc on peut pas se positioner juste apres une list et en general sa ajoute la balise apres un autre element
        // TODO: refacto cet fonction
        //       deplacer l'ajout de cet balise au bon endroit
        //       gerer le probleme de cursor ++ ou --
        //       liste dans list
        //       peut etre lire le livre sur ecrire un interpreteur avant d'essayer de lexer mes couilles comme ça
        if (md->list_item_added && md->in[md->in_cursor] == '\n') {
            md->out_cursor--;
            md_append(md, "</ul>\n");
        }
    }
    md->in_cursor--;
    return md->in_cursor - before;
}

int skip_line(MegaData *md)
{
    int before = md->in_cursor;
    while (CHECK_MD_BOUND(md) && md->in[md->in_cursor] != '\n') {
        md->out[md->out_cursor] = md->in[md->in_cursor];
        md->in_cursor++;
        md->out_cursor++;
    }
    return md->in_cursor - before;
}

int skip_endline(MegaData *md)
{
    md->out_cursor++;
    int before = md->in_cursor;
    while (CHECK_MD_BOUND(md) && md->in[md->in_cursor] != '\n') {
        md->out[md->out_cursor] = md->in[md->in_cursor];
        md->in_cursor++;
        md->out_cursor++;
    }
    md->out_cursor--;
    return md->in_cursor - before;
}

void repeat(MegaData *md, const char c, const int nb)
{
    for (int i=0; i<nb; i++) {
        md->out[++md->out_cursor] = c;
    }
}

int mmdeeznut(MegaData *md)
{
    assert(md->in_cap < md->out_cap && "output buffer should be bigger than the input file size.");
    md->list_item_added = 0;
    while (CHECK_MD_BOUND(md)) {
        int begin_skipped = skip_begin(md);
        md->in_cursor++;
        if ((md->in[md->in_cursor] == '-' || md->in[md->in_cursor] == '+') && md->in[md->in_cursor+1] == ' ') {
            md->list_item_added = 1;
            if (md->list_level < begin_skipped) {
                md->out_cursor--;
                md_append(md, "<ul>\n");
                repeat(md, ' ', begin_skipped);
            }
            md->list_level = begin_skipped;
            md->in_cursor += 2;
            md->out_cursor--;
            md_append(md, "<li>");
            skip_endline(md);
            md_append(md, "</li>");
            md->out_cursor++;
        } else if (md->list_item_added) {
            md->list_item_added = 0;
            // NOTE: voir note dans skip_begin()
            // md->out_cursor--;
            // md_append(md, "</ul>\n");
            // repeat(md, ' ', begin_skipped);
            // md->out_cursor++;
        }
        if (md->in[md->in_cursor] == '#') {
            int level = 1;
            while (md->in[md->in_cursor+level] == '#' && level < 6)
                level++;

            if (md->in[md->in_cursor+level] == ' ') {
                md->in_cursor += level+1;

                char open[] = "<h0>";
                open[2] += level;
                md->out_cursor--;
                md_append(md, open);
                skip_endline(md);
                char close[] = "</h0>";
                close[3] += level;
                md_append(md, close);
                md->out_cursor++;
            }
        }
        skip_line(md);
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
    };
    int data_size = mmdeeznut(&md);

    printf("%s", mmd_data);
    printf("--------------------\n");
    printf("%.*s", data_size, html_data);
    printf("%d -> %d bytes\n", file_size, data_size);

    fwrite(html_data, data_size, 1, output_file);

    fclose(output_file);
    fclose(input_file);
    return 0;
}
