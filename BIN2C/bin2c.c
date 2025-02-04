#include <stdio.h>
#include <stdlib.h>

int bin2c(void *binary_data, size_t binary_size, FILE *out, const char *array_name)
{
    unsigned char *data;
    unsigned char *data_end;

    data = binary_data;
    data_end = data + binary_size;

    fprintf(out, "const unsigned char %s[%zu] = {", array_name, binary_size);

    while (data != data_end) {
        fprintf(out, "%hhu,\n", *data);
        data++;
    }

    fprintf(out, "};");

    return 0;
}

int main(void)
{
    FILE *input_file;
    int input_size;
    void *input_data;
    FILE *output_file;
    
    input_file = fopen("res", "rb");
    if (!input_file) {
        return -1;
    }
    fseek(input_file, 0, SEEK_END);
    input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    input_data = malloc(input_size);
    if (!input_data) {
        return -1;
    }
    fread(input_data, input_size, 1, input_file);
    fclose(input_file);

    output_file = fopen("res.c", "wb");
    if (!output_file) {
        return -1;
    }

    bin2c(input_data, input_size, output_file, "res_data");
    
    fclose(output_file);

    free(input_data);
}
