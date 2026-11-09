#include <stdio.h>

int main(void)
{
    FILE *file = fopen("sample.txt", "rb");
    if (file == NULL) {
        perror("sample.txt");
        return 1;
    }

    int ch = fgetc(file);
    if (ch == EOF) {
        fprintf(stderr, "Could not read the first byte\n");
        fclose(file);
        return 1;
    }

    printf("First byte: %c\n", ch);
    printf("Position after reading it: %ld\n", ftell(file));

    if (fseek(file, 3, SEEK_SET) != 0) {
        perror("fseek");
        fclose(file);
        return 1;
    }

    ch = fgetc(file);
    if (ch == EOF) {
        fprintf(stderr, "Could not read after seeking\n");
        fclose(file);
        return 1;
    }

    printf("Byte at position 3: %c\n", ch);
    fclose(file);
    return 0;
}