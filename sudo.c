#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 9
#define MAX_LEADERBOARD 1000
#define MAX_NAME_LEN 50

int difficulty;

typedef struct {
    clock_t start_time;
    clock_t stop_time;
    int running;
} Stopwatch;

Stopwatch stopwatch;

void stopwatch_start(Stopwatch* sw) {
    sw->start_time = clock();
    sw->running = 1;
    printf("The game has begun\n");
}

void stopwatch_stop(Stopwatch* sw) {
    if (sw->running) {
        sw->stop_time = clock();
        sw->running = 0;
        printf("The Game has ended.\n");
        int duration = (int)((sw->stop_time - sw->start_time) / CLOCKS_PER_SEC);
        printf("Total time spent: %d seconds\n", duration);
    } else {
        printf("Stopwatch is not running!\n");
    }
}

void stopwatch_elapsed(Stopwatch* sw) {
    if (sw->running) {
        clock_t current_time = clock();
        int duration = (int)((current_time - sw->start_time) / CLOCKS_PER_SEC);
        printf("Total time: %d seconds\n", duration);
    }
}

int stopwatch_getStopTime(Stopwatch* sw) {
    if (!sw->running) {
        return (int)((sw->stop_time - sw->start_time) / CLOCKS_PER_SEC);
    } else {
        printf("Stopwatch is still running!\n");
        return -1;
    }
}

void showRules() {
    printf("Welcome to Sudoku! This game offers four difficulty levels:\n");
    printf("1. Easy - Score: 1000\n");
    printf("2. Medium - Score: 2000\n");
    printf("3. Hard - Score: 3000\n");
    printf("4. Extremely Difficult - Score: 5000\n");

    printf("\nFeatures and Penalties:\n");
    printf("1. **Mistake Penalty**: Placing a number that already exists in the row, column, or sub-grid.\n");
    printf("   - **Score Deduction**: 5 points\n");
    printf("2. **Rewrite Penalty**: Editing a previously placed number.\n");
    printf("   - **Score Deduction**: 20 points\n");
    printf("3. **Debug Option**: Removes incorrectly placed numbers by comparing them with the solution.\n");
    printf("   - **Score Deduction**: Each time increases by 100 (100, 200, 300....)\n");
    printf("4. **Lifelines**: Replaces the most difficult cell with the correct number. Can be used only once\n");
    printf("   - **Score Deduction**: 10%% of total\n");
    printf("5. **Time Penalty**: A penalty is applied for every minute spent.\n");
    printf("   - **Score Deduction**: 100 points per minute\n");

    printf("\nBonus System:\n");
    printf("Any unused moves at the end of the game will be converted into bonus points.\n");
    printf("For instance, if 10 moves remain, the bonus will be calculated as the sum of the numbers from 1 to 10 (i.e., 1 + 2 + 3 + ... + 10).\n");
}

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
    int position;
} LeaderboardEntry;

int compare_leaderboard(const void* a, const void* b) {
    LeaderboardEntry* entry_a = (LeaderboardEntry*)a;
    LeaderboardEntry* entry_b = (LeaderboardEntry*)b;
    return entry_b->score - entry_a->score;
}

int read_leaderboard(LeaderboardEntry* leaderboard, const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;

    if (!file) {
        fprintf(stderr, "Error: Unable to open the leaderboard file.\n");
        return 0;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header

    while (fgets(line, sizeof(line), file) && count < MAX_LEADERBOARD) {
        int position;
        char name[MAX_NAME_LEN];
        int score;

        sscanf(line, "%d %s %d", &position, name, &score);
        leaderboard[count].position = position;
        strcpy(leaderboard[count].name, name);
        leaderboard[count].score = score;
        count++;
    }

    fclose(file);
    return count;
}

void write_leaderboard(LeaderboardEntry* leaderboard, int count, const char* filename) {
    FILE* file = fopen(filename, "w");

    if (!file) {
        fprintf(stderr, "Error: Unable to open the leaderboard file.\n");
        return;
    }

    fprintf(file, "No Name Score\n");

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d %s %d\n", leaderboard[i].position, leaderboard[i].name, leaderboard[i].score);
    }

    fclose(file);
}

void update_leaderboard(const char* name, int score, const char* filename) {
    LeaderboardEntry leaderboard[MAX_LEADERBOARD];
    int count = read_leaderboard(leaderboard, filename);

    if (count < MAX_LEADERBOARD) {
        strcpy(leaderboard[count].name, name);
        leaderboard[count].score = score;
        leaderboard[count].position = 0;
        count++;
    }

    qsort(leaderboard, count, sizeof(LeaderboardEntry), compare_leaderboard);

    for (int i = 0; i < count; i++) {
        leaderboard[i].position = i + 1;
    }

    write_leaderboard(leaderboard, count, filename);

    printf("Leaderboard updated successfully!\n");
}

void get_player_name(int score) {
    char player_name[MAX_NAME_LEN];
    int player_score;

    printf("Enter your name: ");
    scanf("%s", player_name);

    player_score = score;

    const char* filename = "leaderboard.txt";
    printf("Name: %s\t Score: %d\n", player_name, player_score);
    update_leaderboard(player_name, player_score, filename);
}

int L;
int puzzle[N][N], locked[N][N], solution[N][N], score;

void fillBasePattern() {
    int base = 3;
    int nums[N];
    for (int i = 0; i < N; i++) {
        nums[i] = i + 1;
    }

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            solution[row][col] = nums[(col + (row * base + row / base) % N) % N];
        }
    }
}

void swap_values(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void swapRows(int row1, int row2) {
    for (int col = 0; col < N; col++) {
        swap_values(&solution[row1][col], &solution[row2][col]);
    }
}

void swapCols(int col1, int col2) {
    for (int row = 0; row < N; row++) {
        swap_values(&solution[row][col1], &solution[row][col2]);
    }
}

void shuffleRows() {
    int base = 3;
    for (int block = 0; block < base; block++) {
        int startRow = block * base;
        for (int i = 0; i < L; i++) {
            int row1 = startRow + rand() % base;
            int row2 = startRow + rand() % base;
            if (row1 != row2)
                swapRows(row1, row2);
        }
    }
}

void shuffleCols() {
    int base = 3;
    for (int block = 0; block < base; block++) {
        int startCol = block * base;
        for (int i = 0; i < L; i++) {
            int col1 = startCol + rand() % base;
            int col2 = startCol + rand() % base;
            if (col1 != col2)
                swapCols(col1, col2);
        }
    }
}

void shuffleRowBlocks() {
    int base = 3;
    for (int i = 0; i < L; i++) {
        int block1, block2;
        do {
            block1 = rand() % base;
            block2 = rand() % base;
        } while (block1 == block2);

        for (int row = 0; row < base; row++) {
            swapRows(block1 * base + row, block2 * base + row);
        }
    }
}

void shuffleColBlocks() {
    int base = 3;
    for (int i = 0; i < L; i++) {
        int block1, block2;
        do {
            block1 = rand() % base;
            block2 = rand() % base;
        } while (block1 == block2);

        for (int col = 0; col < base; col++) {
            swapCols(block1 * base + col, block2 * base + col);
        }
    }
}

void makeSolutionSudoku() {
    fillBasePattern();
    shuffleRows();
    shuffleCols();
    shuffleRowBlocks();
    shuffleColBlocks();
}

void printPuzzle() {
    printf(" -----------------------\n");
    for (int i = 0; i < 9; i++) {
        printf("| ");
        for (int j = 0; j < 9; j++) {
            printf("%d ", puzzle[i][j]);
            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf(" -----------------------\n");
    }
}

void printLocked() {
    printf(" -----------------------\n");
    for (int i = 0; i < 9; i++) {
        printf("| ");
        for (int j = 0; j < 9; j++) {
            printf("%d ", locked[i][j]);
            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf(" -----------------------\n");
    }
}

void printSolution() {
    printf(" -----------------------\n");
    for (int i = 0; i < N; i++) {
        printf("| ");
        for (int j = 0; j < N; j++) {
            printf("%d ", solution[i][j]);
            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf(" -----------------------\n");
    }
}

void debugPuzzle() {
    printf("Debugging: Checking for incorrect values...\n");

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (puzzle[i][j] != 0 && puzzle[i][j] != solution[i][j]) {
                printf("Incorrect value at (%d, %d) removed.\n", i + 1, j + 1);
                puzzle[i][j] = 0;
            }
        }
    }

    printf("Debugging complete.\n");
}

int checkCol(int row, int col, int num) {
    for (int i = 0; i < 9; i++) {
        if (puzzle[row][i] == num) {
            printf("Value already exists in the row\n");
            return 0;
        }
    }
    return 1;
}

int checkRow(int row, int col, int num) {
    for (int i = 0; i < 9; i++) {
        if (puzzle[i][col] == num) {
            printf("Value already exists in the column\n");
            return 0;
        }
    }
    return 1;
}

int checkSquare(int row, int col, int num) {
    row = (row / 3) * 3;
    col = (col / 3) * 3;
    for (int i = row; i < row + 3; i++) {
        for (int j = col; j < col + 3; j++) {
            if (puzzle[i][j] == num) {
                printf("Value already exists in the SubSquare\n");
                return 0;
            }
        }
    }
    return 1;
}

int isValid(int row, int col, int num) {
    int s, c, r;
    r = checkRow(row, col, num);
    c = checkCol(row, col, num);
    s = checkSquare(row, col, num);
    return s && c && r;
}

void copySolutionToPuzzle() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            locked[i][j] = puzzle[i][j] = solution[i][j];
        }
    }
}

void generatePuzzle() {
    printf("Welcome to Sudoku!\nGood luck—aim for the top of the leaderboard!\n");
    showRules();
    int remove;
    int validInput = 0;

    do {
        printf("Choose Difficulty Level:\n");
        printf("1. Easy\n");
        printf("2. Medium\n");
        printf("3. Hard\n");
        printf("4. Extremely Difficult\n");
        printf("Enter a number (1-4): ");

        if (scanf("%d", &difficulty) != 1) {
            printf("Invalid input! Please enter a number between 1 and 4.\n");
            while (getchar() != '\n'); // Clear input buffer
        } else if (difficulty < 1 || difficulty > 4) {
            printf("Invalid input! Please choose a difficulty between 1 and 4.\n");
        } else {
            validInput = 1;
        }
    } while (!validInput);

    if (difficulty == 1) {
        remove = 41;
        L = 5;
        printf("Difficulty chosen: Easy\n");
    } else if (difficulty == 2) {
        remove = 46;
        L = 8;
        printf("Difficulty chosen: Medium\n");
    } else if (difficulty == 3) {
        remove = 51;
        L = 10;
        printf("Difficulty chosen: Hard\n");
    } else {
        remove = 61;
        L = 20;
        printf("Difficulty chosen: Extremely Difficult\n");
    }
    makeSolutionSudoku();
    copySolutionToPuzzle();

    srand(time(NULL));
    while (remove > 0) {
        int i = rand() % N;
        int j = rand() % N;
        if (puzzle[i][j] != 0) {
            locked[i][j] = puzzle[i][j] = 0;
            remove--;
        }
    }
}

void useLifeline() {
    int mostDifficultRow = -1, mostDifficultCol = -1, maxCandidates = -1;

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (puzzle[row][col] == 0) {
                int candidates = 0;
                int possible[10] = {0};

                for (int i = 0; i < N; i++) {
                    if (puzzle[row][i] != 0) {
                        possible[puzzle[row][i]] = 1;
                    }
                }

                for (int i = 0; i < N; i++) {
                    if (puzzle[i][col] != 0) {
                        possible[puzzle[i][col]] = 1;
                    }
                }

                int startRow = (row / 3) * 3, startCol = (col / 3) * 3;
                for (int i = startRow; i < startRow + 3; i++) {
                    for (int j = startCol; j < startCol + 3; j++) {
                        if (puzzle[i][j] != 0) {
                            possible[puzzle[i][j]] = 1;
                        }
                    }
                }

                for (int num = 1; num <= 9; num++) {
                    if (!possible[num]) {
                        candidates++;
                    }
                }

                if (candidates > maxCandidates) {
                    maxCandidates = candidates;
                    mostDifficultRow = row;
                    mostDifficultCol = col;
                }
            }
        }
    }

    if (mostDifficultRow != -1 && mostDifficultCol != -1) {
        puzzle[mostDifficultRow][mostDifficultCol] = solution[mostDifficultRow][mostDifficultCol];
        printf("Lifeline used! The cell at (%d, %d) has been filled with the correct number.\n",
               mostDifficultRow + 1, mostDifficultCol + 1);
    } else {
        printf("No empty cells found to use the lifeline!\n");
    }
}

int calculateScore(int movesLeft, int difficulty, int time, int debugCount, int lifelineUsed, int mistakes, int rewrites) {
    int baseScore = difficulty * 1000;
    if (difficulty == 4)
        baseScore = 5000;

    int finalScore = baseScore;

    printf("\nScore Calculation:\n");
    printf("Base score: %d\n", baseScore);

    int mistakePenalty = mistakes * 5;
    printf("Mistake penalty: -%d (%d mistakes)\n", mistakePenalty, mistakes);
    finalScore -= mistakePenalty;

    int rewritePenalty = rewrites * 20;
    printf("Rewrite penalty: -%d (%d rewrites)\n", rewritePenalty, rewrites);
    finalScore -= rewritePenalty;

    int debugPenalty = 100 * (debugCount * (debugCount + 1)) / 2;
    printf("Debug penalty: -%d (%d uses)\n", debugPenalty, debugCount);
    finalScore -= debugPenalty;

    int timePenalty = (time / 60) * 100;
    printf("Time penalty: -%d (%d minutes)\n", timePenalty, time / 60);
    finalScore -= timePenalty;

    int moveBonus = (movesLeft * (movesLeft + 1)) / 2;
    printf("Move bonus: +%d (%d moves left)\n", moveBonus, movesLeft);
    finalScore += moveBonus;

    if (lifelineUsed) {
        int lifelinePenalty = finalScore / 10;
        printf("Lifeline penalty: -%d (10 percent of total)\n", lifelinePenalty);
        finalScore -= lifelinePenalty;
    } else {
        printf("Lifeline penalty: 0 (not used)\n");
    }

    if (finalScore < 0)
        finalScore = 0;
    printf("Final Score: %d\n", finalScore);
    return finalScore;
}

void playSudoku() {
    printf("Start solving the Sudoku!\n");
    stopwatch_start(&stopwatch);
    int move = 81;
    int debug = 0;
    int ll = 1;
    int mistake = 0;
    int rewrite = 0;
    int time;
    while (move--) {
        printf("Remaining moves: %d\n", move + 1);
        stopwatch_elapsed(&stopwatch);
        printPuzzle();

        printf("Enter your move in the format: row col num (e.g., 2 5 5)\n");
        if (ll)
            printf("Type 0 to exit.\t Type -1 to see rules.\t -2 for debugging.\t -3 for LifeLine\n");
        else
            printf("Type 0 to exit.\t Type -1 to see rules.\t -2 for debugging.\n");

        int row, col, num;
        scanf("%d", &row);

        if (row == 0) {
            printf("Do you want to quit? (y to quit, c to cancel): ");
            char exit_char;
            scanf(" %c", &exit_char);
            if (exit_char == 'y') {
                printf("Thanks for playing!\n");
                break;
            } else {
                move++;
                continue;
            }
        }

        if (row == -1) {
            showRules();
            move++;
            continue;
        }

        if (row == -2) {
            debugPuzzle();
            move++;
            debug++;
            continue;
        }

        if (row == -3) {
            if (ll) {
                useLifeline();
                ll = 0;
            } else {
                printf("Lifeline already used! You can only use it once.\n");
            }
            continue;
        }

        scanf("%d %d", &col, &num);
        row--;
        col--;

        if (row < 0 || row >= N || col < 0 || col >= N || num < 1 || num > 9) {
            printf("Invalid input! Please enter valid row, column, and number.\n");
            move++;
            continue;
        }

        if (locked[row][col] != 0) {
            printf("This cell can't be modified. Try another move.\n");
            printf("Here is the Locked Numbers:\n");
            printLocked();
            move++;
            continue;
        }

        if (!isValid(row, col, num)) {
            printf("Invalid move! This number violates Sudoku rules.\n");
            mistake++;
            continue;
        }

        if (puzzle[row][col] != 0) {
            printf("Rewriting the cell\n");
            rewrite++;
        }

        puzzle[row][col] = num;

        if (memcmp(puzzle, solution, sizeof(solution)) == 0) {
            printPuzzle();
            printf("Congratulations! You have solved the Sudoku!\n");
            printf("Here is the solution:\n");
            printSolution();
            stopwatch_stop(&stopwatch);
            time = stopwatch_getStopTime(&stopwatch);
            int score = calculateScore(move, difficulty, time, debug, ll, mistake, rewrite);
            get_player_name(score);
            break;
        }
    }
}

int main() {
    char ans;
    do {
        generatePuzzle();
        playSudoku();
        printf("Do you want to play another match? (y to play, n to quit): ");
        scanf(" %c", &ans);
    } while (ans == 'y');

    return 0;
}
