#include <stdio.h>
#include <string.h>

int max(int a, int b) { 
    if(a > b) 
        return a; 
    else 
        return b;
}

int main() {
    char a[100], b[100];
    FILE *file1 = fopen("text1.txt", "r");
    if(file1 == NULL) {
        return 1;
    }
    int i = 0;
    char ch;
    while((ch = fgetc(file1)) != EOF && i < 99) {
        a[i++] = ch;
    }
    a[i] = '\0';
    fclose(file1);
    FILE *file2 = fopen("text2.txt", "r");
    if(file2 == NULL) {
        return 1;
    }
    i = 0;
    while((ch = fgetc(file2)) != EOF && i < 99) {
        b[i++] = ch;
    }
    b[i] = '\0';
    fclose(file2);

    printf("test\n");

    return 0;
}
