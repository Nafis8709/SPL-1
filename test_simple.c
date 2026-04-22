#include <stdio.h>
#include <string.h>

int findMax(int *arr, int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

void printArray(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    int arr[] = {5, 2, 8, 1, 9, 3};
    int n = 6;
    
    int max = findMax(arr, n);
    printf("Max: %d\n", max);
    
    printArray(arr, n);
    
    return 0;
}
