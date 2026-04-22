#include <stdio.h>

#define MAX 100

int bfs(int n, int source, int destination, int graph[MAX][MAX], int parent[MAX]) {
    int visited[MAX];
    int queue[MAX];
    int front = 0, rear = 0;
    
    
    for (int i = 0; i < MAX; i++) {
        visited[i] = 0;
    }
    
    queue[rear++] = source;
    visited[source] = 1;
    parent[source] = -1;
    
    while (front < rear) {
        int u = queue[front++];
        
        for (int v = 1; v <= n; v++) {
            if (!visited[v] && graph[u][v] > 0) {
                queue[rear++] = v;
                parent[v] = u;
                visited[v] = 1;
                
                if (v == destination) {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

/*int main() {
    int n, source, destination, edge;
    int graph[MAX][MAX];
    int parent[MAX];
    
    scanf("%d", &n);
    
    
    scanf("%d %d %d", &source, &destination, &edge);
    
    
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            graph[i][j] = 0;
        }
    }
    
    
    for (int i = 0; i < edge; i++) {
        int from, to, bandwidth;
        scanf("%d %d %d", &from, &to, &bandwidth);
        
        
        graph[from][to] += bandwidth;
        graph[to][from] += bandwidth;
    }
    
    
    int totalFlow = 0;
    
    while (bfs(n, source, destination, graph, parent)) {
        
        int minCapacity = 999;
        for (int v = destination; v != source; v = parent[v]) {
            int u = parent[v];
            if (graph[u][v] < minCapacity) {
                minCapacity = graph[u][v];
            }
        }
        
        
        for (int v = destination; v != source; v = parent[v]) {
            int u = parent[v];
            graph[u][v] -= minCapacity;
            graph[v][u] += minCapacity;
        }
        
        totalFlow += minCapacity;
    }
    
    
    printf("\nThe bandwidth is %d\n", totalFlow);
    
    return 0;
}*/
