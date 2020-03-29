/*   Copyright (c) 2010, 2019 by Kent D. Lee */
/*   This tool is free to use for educational and commercial purposes.
     If you wish to incorporate this code, in full or in part, into a
     commercial product then you must obtain a license from the author. */
/*   Added support for more components */
/*   now allow components to be defined anywhere in source file using
     the component keyword */
/*   Allow isolation of one or more components using the isolate
     keyword. This removes all components that to not (transitively)
     touch the component(s) being isolated */
/*   Compile this code with the gnu C compiler using this command.

         gcc -o srmstofigs srmstofigs.c

     If you have trouble compiling on a Mac OS X system because stdlib.h
     is missing, you need to make sure you have the command-line tools
     installed. You can do that with

     sudo xcode-select --install

     If they are already installed, you may need to link to the include
     directory. That might be done by compiling

         gcc -o srmstofigs -I /path/to/stdlib.h srmstofigs.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ppi 1200
#define max_components 5000
#define max_stack 100

int line;
char component[max_components][25];
char component_name[max_components][50];
char mark[max_components];
char stack[max_stack];
char emptyString[] = "";
char spacenewline[] = " \n";
char space[] = " ";
char doublequote[] = "\"";

int num_components;
int tos = 0;
char *strrtrim(char *s);
char *next_token(char *s, char *ct);
void push(int n);

int getout(int i) {
    if (i == 1) printf("srmstofigs: Invalid file format on line %d\n", line);
    if (i == 2) printf("srmstofigs: Too many components\n");
    if (i == 3) printf("Usage: srmstofigs <srms filename>\n");
    if (i == 4) printf("srmstofigs: Output file(s) already exist\n");
    if (i == 5) printf("srmstofigs: Undefined component on line %d\n", line);
    if (i == 6) printf("srmstofigs: Isolation Stack overflow\n");
    if (i == 7) printf("srmstofigs: Isolation Stack underflow\n");
    exit(0);
}

char *next_token(char *s, char *ct) {
    char *rs;
    rs = strtok(s, ct);
    if (rs != NULL) {
        if (!strcmp(rs = strrtrim(rs), "")) rs = strrtrim(strtok(NULL, ct));
    } else
        rs = emptyString;
    return (rs);
}

void push(int n) {
    if (tos == max_stack) getout(6);
    stack[tos++] = n;
}

int isEmpty() { return tos == 0; }

int pop() {
    if (isEmpty()) getout(7);
    return stack[--tos];
}

char *strrtrim(char *s) {
    int i = strlen(s) - 1;
    while (i >= 0) {
        if (s[i] == ' ')
            s[i] = '\0';
        else
            return (s);
        i = i - 1;
    }
    return (s);
}

int compidx(char *compid) {
    int i = num_components - 1;
    while (i >= 0)
        if (!strcmp(component[i--], compid)) return (i + 1);
    getout(5);
    return -1;
}

int icompidx(char *compid) { /* for isolated compidx */
    int i;
    int c = 0;

    for (i = 0; i < num_components; i++)
        if (!strcmp(component[i], compid))
            return c;
        else if (mark[i])
            c++;

    getout(5);
    return -1;
}

int main(int argc, char *argv[]) {
    FILE *fp;
    FILE *ofp;
    char fileout[256];
    int filenum = 0;
    int state = 0;
    int isolate = 0;
    int markCount = 0;
    char s[256];
    char *token, *token1, *token2;
    int position;
    float width, height;
    int pwidth, pheight;
    int k, l;
    int vpos, hpos, hpos2, voffset;

    if (argc != 2) getout(3);
    fp = fopen(argv[1], "r");
    if (fp == NULL) printf("srmstofigs: %s file not found\n", argv[1]);

    /* find the components */
    state = 0;
    line = 0;
    num_components = 0;
    while (fgets(s, 255, fp) != NULL) {
        line = line + 1;
        if ((s[0] != '\n') && (s[0] != '#')) {
            switch (state) {
                case 0:
                    token = next_token(s, spacenewline);
                    if (sscanf(token, "%f", &width) == EOF) getout(1);
                    if ((token = next_token(NULL, spacenewline)) == NULL)
                        getout(1);
                    if (sscanf(token, "%f", &height) == EOF) getout(1);
                    pwidth = ppi * width;
                    pheight = ppi * height;
                    state = 1;
                    break;
                case 1:
                    token = next_token(s, spacenewline);
                    if (strcmp(token, "begin")) state = 3;
                    state = 2;
                    break;
                case 2:
                    token = next_token(s, spacenewline);
                    if (num_components == max_components) getout(2);
                    if (!strcmp(token, "end;")) {
                        state = 3;
                        vpos = pheight;
                    } else {
                        strcpy(component[num_components], token);
                        if ((token = next_token(NULL, doublequote)) == NULL)
                            getout(1);
                        strcpy(component_name[num_components], token);
                        if (!isolate) {
                            mark[num_components] = 1;
                            markCount++;
                        } else
                            mark[num_components] = 0;
                        num_components = num_components + 1;
                    }
                    break;
                case 3:
                    token = next_token(s, spacenewline);
                    if (!strcmp(token, "component")) {
                        token1 = next_token(NULL, space);
                        strcpy(component[num_components], token1);
                        if ((token2 = next_token(NULL, doublequote)) == NULL)
                            getout(1);
                        strcpy(component_name[num_components], token2);
                        if (!isolate) {
                            mark[num_components] = 1;
                            markCount++;
                        } else
                            mark[num_components] = 0;
                        num_components = num_components + 1;
                    } else if (!strcmp(token, "isolate")) {
                        token1 = next_token(NULL, spacenewline);
                        printf("isolation of %s requested\n", token1);
                        if (!isolate) {
                            for (k = 0; k < num_components; k++) mark[k] = 0;
                            markCount = 0;
                            isolate = 1;
                        }
                        mark[compidx(token1)] = 1;
                        markCount++;
                        push(compidx(token1));
                    }
                    break;
            }
        }
    }

    /* isolate any components that were requested */
    while (!isEmpty()) {
        int current = pop();
        state = 0;
        line = 0;
        fp = fopen(argv[1], "r");
        while (fgets(s, 255, fp) != NULL) {
            line = line + 1;
            if ((s[0] != '\n') && (s[0] != '#')) {
                switch (state) {
                    case 0:
                        token = next_token(s, spacenewline);
                        if (sscanf(token, "%f", &width) == EOF) getout(1);
                        if ((token = next_token(NULL, spacenewline)) == NULL)
                            getout(1);
                        if (sscanf(token, "%f", &height) == EOF) getout(1);
                        pwidth = ppi * width;
                        pheight = ppi * height;
                        state = 1;
                        break;
                    case 1:
                        token = next_token(s, spacenewline);
                        if (strcmp(token, "begin")) state = 3;
                        state = 2;
                        break;
                    case 2:
                        token = next_token(s, spacenewline);
                        if (!strcmp(token, "end;")) {
                            state = 3;
                            vpos = pheight;
                        }
                        break;
                    case 3:
                        token = next_token(s, spacenewline);
                        token1 = next_token(NULL, space);
                        if (!strcmp(token, ";")) {                /* ignore */
                        } else if (!strcmp(token, "ltext@")) {    /* ignore */
                        } else if (!strcmp(token, "ctext@")) {    /* ignore */
                        } else if (!strcmp(token, "rtext@")) {    /* ignore */
                        } else if (!strcmp(token, "component")) { /*ignore*/
                        } else if (!strcmp(token, "isolate")) {   /*ignore*/
                        } else {                                  /* srm */
                            k = compidx(token);
                            l = compidx(token1);
                            if ((k == current) && !mark[l]) {
                                mark[l] = 1;
                                push(l);
                                markCount++;
                            }
                            if ((l == current) && !mark[k]) {
                                mark[k] = 1;
                                push(k);
                                markCount++;
                            }
                        }
                        break;
                }
            }
        }
    }

    /* generate the output */
    printf("Generating Output\n");
    state = 0;
    line = 0;
    vpos = 0;
    pheight = 1;
    fp = fopen(argv[1], "r");
    while (fgets(s, 255, fp) != NULL) {
        line = line + 1;
        if ((s[0] != '\n') && (s[0] != '#')) {
            if (vpos >= pheight) {
                filenum = filenum + 1;
                sprintf(fileout, "%s%d.fig", argv[1], filenum);
                ofp = fopen(fileout, "r");
                if (ofp != NULL) getout(4);
                ofp = fopen(fileout, "w");
                fprintf(ofp, "#FIG 3.1\n");
                fprintf(ofp, "Portrait\n");
                fprintf(ofp, "Center\n");
                fprintf(ofp, "Inches\n");
                fprintf(ofp, "1200 2\n");
                vpos = 150;
                l = 0;
                for (k = 0; k < num_components; k++) {
                    if (mark[k]) {
                        hpos = (l + 1) * pwidth / (markCount + 1);
                        fprintf(
                            ofp,
                            "4 0 -1 0 0 0 10 -1.571 4 105 %d %d %d %s\\001\n",
                            pheight, hpos + 100, vpos, component_name[k]);
                        fprintf(ofp,
                                "2 1 0 1 -1 7 0 0 -1 0.000 0 0 -1 0 0 2\n");
                        fprintf(ofp, "      %d %d %d %d\n", hpos, vpos, hpos,
                                pheight);
                        l++;
                    }
                }
                vpos = vpos + 1800;
            }

            switch (state) {
                case 0:
                    token = next_token(s, spacenewline);
                    if (sscanf(token, "%f", &width) == EOF) getout(1);
                    if ((token = next_token(NULL, spacenewline)) == NULL)
                        getout(1);
                    if (sscanf(token, "%f", &height) == EOF) getout(1);
                    pwidth = ppi * width;
                    pheight = ppi * height;
                    state = 1;
                    break;
                case 1:
                    token = next_token(s, spacenewline);
                    if (strcmp(token, "begin")) state = 3;
                    state = 2;
                    break;
                case 2:
                    token = next_token(s, spacenewline);
                    if (!strcmp(token, "end;")) {
                        state = 3;
                        vpos = pheight;
                    }
                    break;
                case 3:
                    token = next_token(s, spacenewline);
                    token1 = next_token(NULL, space);
                    token2 = next_token(NULL, doublequote);
                    if (!strcmp(token, ";")) {
                        vpos = vpos + 150;
                    } else if (!strcmp(token, "ltext@")) {
                        if (mark[compidx(token1)]) {
                            k = icompidx(token1);
                            hpos = (k + 1) * pwidth / (markCount + 1);
                            fprintf(ofp,
                                    "4 0 -1 0 0 0 10 0.0000 4 105 %d %d %d "
                                    "%s\\001\n",
                                    pwidth - hpos - 100, hpos + 100, vpos,
                                    token2);
                            voffset = 150;
                            token = next_token(NULL, spacenewline);
                            if (!strcmp(token, ";")) vpos = vpos + voffset;
                        }
                    } else if (!strcmp(token, "ctext@")) {
                        if (mark[compidx(token1)]) {
                            k = icompidx(token1);
                            hpos = (k + 1) * pwidth / (markCount + 1);
                            fprintf(ofp,
                                    "4 1 -1 0 0 0 10 0.0000 4 105 %d %d %d "
                                    "%s\\001\n",
                                    pwidth - hpos, hpos, vpos, token2);
                            voffset = 150;
                            token = next_token(NULL, spacenewline);
                            if (!strcmp(token, ";")) vpos = vpos + voffset;
                        }
                    } else if (!strcmp(token, "rtext@")) {
                        if (mark[compidx(token1)]) {
                            k = icompidx(token1);
                            hpos = (k + 1) * pwidth / (markCount + 1);
                            fprintf(ofp,
                                    "4 2 -1 0 0 0 10 0.0000 4 105 %d %d %d "
                                    "%s\\001\n",
                                    pwidth - hpos - 100, hpos - 100, vpos,
                                    token2);
                            voffset = 150;
                            token = next_token(NULL, spacenewline);
                            if (!strcmp(token, ";")) vpos = vpos + voffset;
                        }
                    } else if (!strcmp(token, "component")) { /*ignore*/
                    } else if (!strcmp(token, "isolate")) {   /*ignore*/
                    } else {                                  /* srm */
                        if (mark[compidx(token)] || mark[compidx(token1)]) {
                            k = icompidx(token);
                            l = icompidx(token1);
                            hpos = (k + l + 2) * pwidth / ((markCount + 1) * 2);
                            if (hpos > (pwidth / 2))
                                fprintf(ofp,
                                        "4 1 -1 0 0 0 10 0.0000 4 105 %d %d %d "
                                        "%s\\001\n",
                                        (pwidth - hpos) * 2, hpos, vpos,
                                        token2);
                            else
                                fprintf(ofp,
                                        "4 1 -1 0 0 0 10 0.0000 4 105 %d %d %d "
                                        "%s\\001\n",
                                        hpos * 2, hpos, vpos, token2);
                            hpos = (k + 1) * pwidth / (markCount + 1);
                            hpos2 = (l + 1) * pwidth / (markCount + 1);
                            token = next_token(NULL, spacenewline);
                            if (strcmp(token, "+")) { //if the token is not +
                              fprintf(ofp,
                                      "2 1 0 1 -1 7 0 0 -1 0.000 0 0 -1 1 0 2\n");
                              fprintf(ofp, "     2 1 1.00 60.00 120.00\n");
                              fprintf(ofp, "      %d %d %d %d\n", hpos, vpos + 50,
                                      hpos2, vpos + 50);
                              voffset = 300;
                            } else {
                              token = ";"; // treat the + like a ; going forward.
                              voffset = 150;
                            }

                            if (!strcmp(token, ";")) vpos = vpos + voffset;
                        }
                    }
                    break;
            }
        }
    }

    return (0);
}
