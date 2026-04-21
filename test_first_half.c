#include <stdio.h>
#include <string.h>

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
    return 0;
}
