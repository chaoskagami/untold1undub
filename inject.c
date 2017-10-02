#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

char* name_map(hpi_file_entry_t *entries, int index, char* blob) {
    return &blob[entries[index].filename_offset];
}

char* data_map(hpi_file_entry_t *entries, int index, char* data) {
    return &data[entries[index].file_offset];
}

int main(int c, char** v) {
    FILE* hpi = fopen(v[1], "rb");
    FILE* hpb = fopen(v[2], "rb");
    if (!hpi)
        die("Failed to open HPI.\n");
    if (!hpb)
        die("Failed to open HPB.\n");

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

        FILE *out = fopen(name, "rb");
        if (!out)
            continue; // This file does not exist; don't replace it.

        fseek(out, 0, SEEK_END);
        size_t fs = ftell(out);
        rewind(out);

        printf("%s [%lu]\n", name, fs);

        data = realloc(data, hpb_file_size + fs);
        fread(&data[hpb_file_size], 1, fs, out);
        fclose(out);

        file_entries[i].file_offset = hpb_file_size;
        file_entries[i].compressed_size = fs;

        hpb_file_size += fs;
    }

    FILE* out_hpb = fopen("out.hpb", "wb");
    fwrite(data, 1, hpb_file_size, out_hpb);
    fclose(out_hpb);

    FILE* out_hpi = fopen("out.hpi", "wb");
    fwrite(header, 1, sizeof(hpi_header_t), out_hpi);
    fwrite(unknown_entries, 1, sizeof(hpi_unknown_entry_t) * header->unknown_entry_count, out_hpi);
    fwrite(file_entries, 1, sizeof(hpi_file_entry_t) * header->file_entry_count, out_hpi);
    fwrite(filename_blob, 1, hpi_file_size - pos, out_hpi);
    fclose(out_hpi);
}
