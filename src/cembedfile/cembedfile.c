#include <stdlib.h>
#include <stdio.h>

static FILE *open_or_exit(const char *fname, const char *mode) {
    FILE *f = fopen(fname, mode);
    if (f == NULL) {
        perror(fname);
        exit(EXIT_FAILURE);
    }
    return f;
}

static size_t get_file_size(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    size_t result = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return result;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr,
                "USAGE: %s {output} {input} {symbol_name}\n\n"
                "  Creates {output} from the contents of {input} using {symbol_name}\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *sym = argv[3];
    FILE *in = open_or_exit(argv[2], "r");
    size_t in_file_size = get_file_size(in);

    FILE *out = open_or_exit(argv[1], "w");

    fprintf(out, "#include <stddef.h>\n");
    fprintf(out, "#include <stdint.h>\n");

    fprintf(out, "\n\n");
    fprintf(out, "const size_t %s_len = %zu;\n\n", sym, in_file_size);
    fprintf(out, "const uint8_t %s[] = {\n    ", sym);

    unsigned char buf[256];
    size_t nread = 0;
    size_t linecount = 0;
    do {
        nread = fread(buf, 1, sizeof(buf), in);
        size_t i;
        for (i = 0; i < nread; i++) {
            fprintf(out, "0x%02x, ", buf[i]);
            if (++linecount == 8) {
                fprintf(out, "\n    ");
                linecount = 0;
            }
        }
    } while (nread > 0);
    if (linecount > 0)
        fprintf(out, "\n");
    fprintf(out, "};\n");

    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;
}
