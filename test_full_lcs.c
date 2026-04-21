#include <stdio.h>
#include <string.h>

int max(int a, int b) { 
    if(a > b) 
        return a; 
    else 
        return b;
}

int LCS(char a[], char b[]) {
   int m = strlen(&a[1]); 
   int n = strlen(&b[1]);

   // int m = strlen(a); 
    //int n = strlen(b);
    int table[m+1][n+1];

    for (int i = 0; i <=m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                table[i][j] = 0;
            else if (a[i-1] == b[j-1])
                table[i][j] = 1+table[i-1][j-1];
            else
                table[i][j] = max(table[i-1][j], table[i][j-1]);
        }
    }

    int index = table[m][n];
    char lcs[index + 1];
    lcs[index] = '\0';

    int i = m, j = n;
    while (i > 0 && j > 0) {
        if (a[i-1] == b[j-1]) {
            lcs[index - 1] = a[i - 1];
            i--;
            j--; 
            index--;
        } 
        else if (table[i - 1][j] > table[i][j - 1]) {
            i--;
        } 
        else {
            j--;
        }
    }
    printf("LCS: %s\n", lcs);

    return table[m][n];
}

int main() {
    return 0;
}
