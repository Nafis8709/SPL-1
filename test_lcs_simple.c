int max(int a, int b) { 
    if(a > b) 
        return a; 
    else 
        return b;
}

int LCS(char a[], char b[]) {
    int m = 5;
    int n = 5;
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
    return table[m][n];
}

int main() {
    return 0;
}
