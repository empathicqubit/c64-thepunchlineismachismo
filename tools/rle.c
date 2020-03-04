#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE *read_file, *write_file;
    unsigned short read_size;
    unsigned char *write_buffer, *read_buffer;

    size_t write_size = 0;
    unsigned char count = 1;
    unsigned char last = 0xFF;
    unsigned char current = 0xFF;
    unsigned short read_index = 0;
    unsigned short write_index = 4; // Skip to the data section

    if(argc < 3) return 1;

    read_file = fopen(argv[1], "rb");
    fseek(read_file, 0L, SEEK_END);
    read_size = ftell(read_file);

    if(!read_size) return 2;

    rewind(read_file);
    read_buffer = malloc(read_size);
    if(!fread(read_buffer, read_size, 1, read_file)) return 3;
    fclose(read_file);

    write_buffer = calloc(1, read_size * 2 + 16); // If every value is different, RLE could make the file size double!

    last = read_buffer[read_index];
    read_index++;

    for(; read_index < read_size; read_index++) {
        current = read_buffer[read_index];
        if(last != current || count == 0xff) {
            write_buffer[write_index] = count;
            write_index++;
            write_buffer[write_index] = last;
            write_index++;
            count = 0;
        }
        count++;
        fprintf(stderr, "C %d L %d C %d W %d\n", current, last, count, write_index);
        last = current;
    }

    write_buffer[write_index] = count;
    write_index++;
    write_buffer[write_index] = current;
    write_index++;

    ((unsigned short*)write_buffer)[0] = (write_index - 4) / 2;
    ((unsigned short*)write_buffer)[1] = read_size;

    write_file = fopen(argv[2], "wb");

    if(!fwrite(write_buffer, write_index, 1, write_file)) return 4;

    fclose(write_file);

    free(write_buffer);
    free(read_buffer);
}
