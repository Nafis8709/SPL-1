int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

int findMaxSum(int arr[], int n, int dp[]) {
    if (n < 0)
        return 0;
    if (n == 0)
        return arr[0];
    int include = arr[n] + findMaxSum(arr, n - 2, dp);
    return include;
}

int main() {
    int arr[] = {1, 2};
    int dp[2];
    int result = findMaxSum(arr, 1, dp);
    return 0;
}
