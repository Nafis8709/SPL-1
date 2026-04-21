#include <stdio.h>
#include <string.h>

int main() {
    char a[100], b[100];
    FILE *file2 = fopen("text2.txt", "r");
    if(file2 == NULL) {
        return 1;
    }
    fclose(file2);
    return 0;
}
