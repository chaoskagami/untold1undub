#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct {
    char magic[4]; // "HPIH"
    uint32_t ukn1;
    uint32_t ukn2;
    uint32_t ukn3;
    uint16_t ukn4;
    uint16_t unknown_entry_count;
    uint16_t file_entry_count;
    uint16_t unknown;
} __attribute__((packed)) hpi_header_t;

typedef struct {
    uint16_t first_file_index;
    uint16_t num_files;
} __attribute__((packed)) hpi_unknown_entry_t;

typedef struct {
    uint32_t filename_offset;
    uint32_t file_offset;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
} __attribute__((packed)) hpi_file_entry_t;

void die(char *error_mesg) {
    fprintf(stderr, "%s", error_mesg);
    exit(1);
}

void mkdir_rec(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

char* dir_name(char* file) {
    int len = strlen(file);
    for(; len > 0; len--) {
        if (file[len] == '/') break;
    }
    char * copy = strdup(file);
    copy[len] = 0;
    return copy;
}

char* name_map(hpi_file_entry_t *entries, int index, char* blob) {
    return &blob[entries[index].filename_offset];
}

char* data_map(hpi_file_entry_t *entries, int index, char* data) {
    return &data[entries[index].file_offset];
}

int main(int c, char** v) {
    if (c < 3 || c > 4) {
        die("usage: extract hpi hpb [flavor]\n  flavor is either 1 (Untold), 5 (EOV), or 0 (Ignore)");
    }

    FILE* hpi = fopen(v[1], "rb");
    FILE* hpb = fopen(v[2], "rb");
    int flavor = 1;

    if (!hpi)
        die("Failed to open HPI.\n");
    if (!hpb)
        die("Failed to open HPB.\n");

    if (c == 4)
        flavor = v[3][0] - '0';
    if (!(flavor == 1 || flavor == 5 || flavor == 0))
        die("invalid flavor\n");

    fseek(hpi, 0, SEEK_END);
    size_t hpi_file_size = ftell(hpi);
    rewind(hpi);

    fseek(hpb, 0, SEEK_END);
    size_t hpb_file_size = ftell(hpb);
    rewind(hpb);

    hpi_header_t *header = malloc(sizeof(hpi_header_t));
    fread(header, 1, sizeof(hpi_header_t), hpi);

    if (strncmp(header->magic, "HPIH", 4))
        die("HPI magic invalid; is this an HPI file?\n");

    hpi_unknown_entry_t *unknown_entries = malloc(sizeof(hpi_unknown_entry_t) * header->unknown_entry_count);
    hpi_file_entry_t    *file_entries    = malloc(sizeof(hpi_file_entry_t)    * header->file_entry_count);

    fread(unknown_entries, 1, sizeof(hpi_unknown_entry_t) * header->unknown_entry_count, hpi);
    fread(file_entries,    1, sizeof(hpi_file_entry_t)    * header->file_entry_count,    hpi);

    size_t pos = ftell(hpi);

    char *filename_blob = malloc(hpi_file_size - pos);
    fread(filename_blob, 1, hpi_file_size - pos, hpi);

    char *data = malloc(hpb_file_size);
    fread(data, 1, hpb_file_size, hpb);

    fclose(hpi);
    fclose(hpb);

    for (int i = 0; i < header->file_entry_count; i++) {
        char *name = name_map(file_entries, i, filename_blob);

        // If you're poking around, comment this. Keep in mind no decompression is performed.
        if (flavor == 1 && strncmp(name, "VOICE/", 6))
            continue; // Not voice files, keep going

        if (flavor == 5 && strncmp(name, "SND/", 4))
            continue; // Not voice files, keep going

        char* dir = dir_name(name);
        mkdir_rec(dir);
        free(dir);

        char *data_off = data_map(file_entries, i, data);

        printf("%s [%hu/%hu]\n", name, file_entries[i].compressed_size, file_entries[i].uncompressed_size);

        FILE *out = fopen(name, "wb");
        if (!out)
            die("Failed to open file to write.\n");
        fwrite(data_off, 1, file_entries[i].compressed_size, out);
        fclose(out);
    }
}
