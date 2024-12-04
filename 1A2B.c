#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMBINATIONS 5040 // Total number of 4-digit unique combinations (10P4 = 5040)
#define DEBUG 0 // Debugging flag for development and testing purposes

// Structure to hold statistical data for the game
typedef struct {
    int totalGames;   // Total number of games played
    int totalRounds;  // Total number of rounds across all games
    int bestGame;     // Minimum number of rounds required to guess correctly
} Stats;

// Utility macros for common operations
#define PRINT_LINE() printf("\n====================\n") // Separator for menu/UI
#define VALID_INPUT(input, length) (strlen(input) == (length) && isValidInput(input, length)) // Validate user input

// Function declarations for modularity and readability
void generateAllCombinations(char combinations[MAX_COMBINATIONS][5], int *count);
void generateRandomAnswer(char *answer, int length);
int isValidInput(const char *input, int length);
void evaluateGuess(const char *answer, const char *guess, int *A, int *B);
void filterCombinations(char combinations[MAX_COMBINATIONS][5], int *count, const char *guess, int A, int B);
int calculateScore(const char *guess, char combinations[MAX_COMBINATIONS][5], int count);
char *selectNextGuess(char combinations[MAX_COMBINATIONS][5], int count);

void playGuessingGame(Stats *stats);
void playAnsweringGame(Stats *stats);
void playWithDifficulty(Stats *stats, int length);
void twoPlayerMode(Stats *stats);
void displayStats(const Stats *stats);

int main() {
    Stats stats = {0, 0, 0}; // Initialize game statistics
    char playAgain;

    do {
        char choice;
        PRINT_LINE();
        printf("1A2B GAME MENU\n");
        printf("a. Computer Guesses (Guessing Mode)\n");
        printf("b. Player Guesses (Answering Mode)\n");
        printf("c. Play with Difficulty\n");
        printf("d. Two Player Mode\n");
        printf("f. View Game Stats\n");
        printf("Enter your choice (a/b/c/d/f): ");
        scanf(" %c", &choice);

        // Dispatch based on user's choice
        if (choice == 'a') playGuessingGame(&stats);
        else if (choice == 'b') playAnsweringGame(&stats);
        else if (choice == 'c') {
            int length;
            printf("Enter difficulty (3/4/5 digits): ");
            scanf("%d", &length);
            playWithDifficulty(&stats, (length < 3 || length > 5) ? 4 : length);
        } else if (choice == 'd') twoPlayerMode(&stats);
        else if (choice == 'f') displayStats(&stats);
        else printf("Invalid choice.\n");

        printf("Play again? (y/n): ");
        scanf(" %c", &playAgain);
    } while (playAgain == 'y' || playAgain == 'Y'); // Loop until user decides to quit

    printf("Goodbye!\n");
    return 0;
}

// Generate all possible unique 4-digit combinations (permutations of 4 digits from 0-9)
// Uses nested loops for exhaustive enumeration
void generateAllCombinations(char combinations[MAX_COMBINATIONS][5], int *count) {
    int a, b, c, d;
    *count = 0; // Initialize combination count
    for (a = 0; a < 10; a++) for (b = 0; b < 10; b++) if (b != a)
    for (c = 0; c < 10; c++) if (c != a && c != b)
    for (d = 0; d < 10; d++) if (d != a && d != b && d != c)
        snprintf(combinations[(*count)++], 5, "%d%d%d%d", a, b, c, d); // Store valid combination
}

// Randomly generate a unique n-digit number (length = n)
// Ensures no repeated digits using a boolean array to track used digits
void generateRandomAnswer(char *answer, int length) {
    int used[10] = {0}; // Boolean array to mark used digits
    int i, digit;
    srand(time(NULL)); // Seed random number generator
    for (i = 0; i < length; i++) {
        do digit = rand() % 10; while (used[digit]); // Retry until an unused digit is found
        used[digit] = 1;
        answer[i] = '0' + digit; // Convert digit to character
    }
    answer[length] = '\0'; // Null-terminate the string
}

// Validate user input for correct length and no repeated digits
int isValidInput(const char *input, int length) {
    int used[10] = {0}; // Boolean array for checking digit repetition
    int i;
    if (strlen(input) != length) return 0; // Check length
    for (i = 0; i < length; i++) {
        if (input[i] < '0' || input[i] > '9' || used[input[i] - '0']) return 0; // Non-digit or duplicate
        used[input[i] - '0'] = 1; // Mark digit as used
    }
    return 1; // Valid input
}

// Evaluate the number of exact matches (A) and partial matches (B) between answer and guess
// "A" denotes correct digit and position; "B" denotes correct digit but wrong position
void evaluateGuess(const char *answer, const char *guess, int *A, int *B) {
    int i, j;
    *A = *B = 0; // Initialize counters
    for (i = 0; i < strlen(answer); i++) {
        if (answer[i] == guess[i]) (*A)++; // Exact match
        else for (j = 0; j < strlen(answer); j++) if (answer[i] == guess[j]) (*B)++; // Partial match
    }
}

// Filter possible combinations based on feedback (A, B)
// Removes combinations that cannot produce the given A and B with the current guess
void filterCombinations(char combinations[MAX_COMBINATIONS][5], int *count, const char *guess, int A, int B) {
    char filtered[MAX_COMBINATIONS][5]; // Array for filtered combinations
    int filteredCount = 0, i;
    for (i = 0; i < *count; i++) {
        int currentA = 0, currentB = 0;
        evaluateGuess(combinations[i], guess, &currentA, &currentB); // Simulate feedback
        if (currentA == A && currentB == B) strcpy(filtered[filteredCount++], combinations[i]); // Keep valid combinations
    }
    if (filteredCount == 0) { // Detect logical inconsistency (cheating)
        printf("YOU CHEAT! Invalid input detected. Game over.\n");
        exit(1);
    }
    *count = filteredCount;
    for (i = 0; i < filteredCount; i++) strcpy(combinations[i], filtered[i]); // Update combinations array
}

// Minimax strategy: Calculate the score of a guess based on the worst-case scenario
// Maximizes the number of remaining valid combinations in the worst case
int calculateScore(const char *guess, char combinations[MAX_COMBINATIONS][5], int count) {
    int scores[5][5] = {0}; // Matrix to track feedback counts
    int maxScore = 0, i, A, B;
    for (i = 0; i < count; i++) {
        evaluateGuess(combinations[i], guess, &A, &B); // Evaluate feedback for current guess
        scores[A][B]++; // Increment feedback occurrence
    }
    for (A = 0; A <= 4; A++) for (B = 0; B <= 4 - A; B++) // Calculate worst-case score
        if (scores[A][B] > maxScore) maxScore = scores[A][B];
    return maxScore;
}

// Minimax strategy: Select the next best guess with the minimum worst-case score
char *selectNextGuess(char combinations[MAX_COMBINATIONS][5], int count) {
    int i, bestScore = MAX_COMBINATIONS + 1, currentScore;
    char *bestGuess = combinations[0];
    for (i = 0; i < count; i++) {
        currentScore = calculateScore(combinations[i], combinations, count); // Compute score for each guess
        if (currentScore < bestScore) { // Update best guess
            bestScore = currentScore;
            bestGuess = combinations[i];
        }
    }
    return bestGuess;
}
// Function for "Guessing Mode" where the computer guesses the player's secret number
void playGuessingGame(Stats *stats) {
    char combinations[MAX_COMBINATIONS][5];
    int totalCombinations = 0, A = 0, B = 0, attempts = 0;

    // Step 1: Generate all possible 4-digit combinations
    generateAllCombinations(combinations, &totalCombinations);
    printf("Think of a 4-digit number (no repeats).\n");

    // Step 2: Iteratively guess based on feedback
    while (A != 4) {
        char *guess = selectNextGuess(combinations, totalCombinations); // Use Minimax strategy for the best guess
        printf("Computer guesses: %s\n", guess);
        printf("Enter '_A_B': ");
        scanf("%d %d", &A, &B); // Input feedback from the user

        // If the computer wins, update stats and break
        if (A == 4) {
            printf("PC WIN in %d attempts!\n", ++attempts);
            stats->totalGames++;
            stats->totalRounds += attempts;
            if (stats->bestGame == 0 || attempts < stats->bestGame)
                stats->bestGame = attempts; // Update best game record
            break;
        }

        // Step 3: Validate feedback for logical consistency
        if (A + B > 4 || A < 0 || B < 0) {
            printf("YOU CHEAT! Invalid input detected. Game over.\n");
            break;
        }

        // Step 4: Filter remaining valid combinations based on feedback
        filterCombinations(combinations, &totalCombinations, guess, A, B);
        attempts++; // Increment the attempt counter
    }
}

// Function for "Answering Mode" where the player guesses the computer's randomly generated number
void playAnsweringGame(Stats *stats) {
    char correctAnswer[5];
    int A = 0, B = 0, attempts = 0;
    char userGuess[20];

    // Step 1: Generate a random 4-digit answer
    generateRandomAnswer(correctAnswer, 4);
    printf("\nSystem generated a 4-digit number.\n");

    // Step 2: Allow the player to guess until they succeed or give up
    while (A != 4) {
        printf("Attempt #%d: Please input your guess (4 unique digits or 'SHOW' to reveal): ", attempts + 1);
        scanf("%s", userGuess);

        // Provide an option to reveal the correct answer
        if (strcasecmp(userGuess, "SHOW") == 0) {
            printf("The correct answer is: %s\n", correctAnswer);
            return; // Exit the game and return to the main menu
        }

        // Validate user input
        if (!VALID_INPUT(userGuess, 4)) {
            printf("Invalid input. Please try again.\n");
            continue;
        }

        // Step 3: Evaluate the player's guess and provide feedback
        evaluateGuess(correctAnswer, userGuess, &A, &B);
        attempts++;

        if (A == 4) {
            printf("YOU WIN! The correct answer was %s.\n", correctAnswer);
            break; // Exit the loop if the player wins
        } else {
            printf("Result: %dA%dB\n", A, B); // Show feedback
        }
    }

    // Step 4: Update statistics
    stats->totalGames++;
    stats->totalRounds += attempts;
    if (stats->bestGame == 0 || attempts < stats->bestGame)
        stats->bestGame = attempts; // Update the best game record
}

// Function for "Difficulty Mode" allowing customization of the number of digits in the game
void playWithDifficulty(Stats *stats, int length) {
    char correctAnswer[6]; // Support up to 5-digit answers
    int A = 0, B = 0, attempts = 0;
    char userGuess[20];

    // Step 1: Generate a random answer of the specified length
    generateRandomAnswer(correctAnswer, length);
    printf("\nSystem generated a %d-digit number.\n", length);

    // Step 2: Allow the player to guess until they succeed or give up
    while (A != length) {
        printf("Attempt #%d: Please input your guess (%d unique digits or 'SHOW' to reveal): ", attempts + 1, length);
        scanf("%s", userGuess);

        // Option to reveal the correct answer
        if (strcasecmp(userGuess, "SHOW") == 0) {
            printf("The correct answer is: %s\n", correctAnswer);
            return; // Exit the game and return to the main menu
        }

        // Validate user input
        if (!VALID_INPUT(userGuess, length)) {
            printf("Invalid input. Please try again.\n");
            continue;
        }

        // Step 3: Evaluate the player's guess and provide feedback
        evaluateGuess(correctAnswer, userGuess, &A, &B);
        attempts++;

        if (A == length) {
            printf("YOU WIN! The correct answer was %s.\n", correctAnswer);
            break; // Exit the loop if the player wins
        } else {
            printf("Result: %dA%dB\n", A, B); // Provide feedback
        }
    }

    // Step 4: Update statistics
    stats->totalGames++;
    stats->totalRounds += attempts;
    if (stats->bestGame == 0 || attempts < stats->bestGame)
        stats->bestGame = attempts; // Update the best game record
}

// Function for "Two Player Mode" where two players compete against each other
void twoPlayerMode(Stats *stats) {
    char player1Answer[6], player2Answer[6];
    int A, B, attempts1 = 0, attempts2 = 0;
    char guess[20];

    // Step 1: Player 1 sets a secret number
    printf("Player 1, enter a 4-digit number (hidden): ");
    scanf("%s", player1Answer);
    if (!VALID_INPUT(player1Answer, 4)) {
        printf("Invalid input. Exiting game.\n");
        return;
    }

    // Step 2: Player 2 guesses Player 1's number
    printf("\nPlayer 2, start guessing!\n");
    A = 0, B = 0;
    while (A != 4) {
        printf("Player 2, your guess (or 'SHOW' to reveal): ");
        scanf("%s", guess);

        if (strcasecmp(guess, "SHOW") == 0) {
            printf("Player 1's answer is: %s\n", player1Answer);
            return; // Exit the game and return to the main menu
        }

        if (!VALID_INPUT(guess, 4)) {
            printf("Invalid input. Try again.\n");
            continue;
        }
        evaluateGuess(player1Answer, guess, &A, &B);
        attempts1++;

        if (A == 4) {
            printf("Player 2 guessed correctly in %d attempts!\n", attempts1);
            break;
        } else {
            printf("Result: %dA%dB\n", A, B);
        }
    }

    // Step 3: Player 2 sets a secret number
    printf("\nPlayer 2, enter a 4-digit number (hidden): ");
    scanf("%s", player2Answer);
    if (!VALID_INPUT(player2Answer, 4)) {
        printf("Invalid input. Exiting game.\n");
        return;
    }

    // Step 4: Player 1 guesses Player 2's number
    printf("\nPlayer 1, start guessing!\n");
    A = 0, B = 0;
    while (A != 4) {
        printf("Player 1, your guess (or 'SHOW' to reveal): ");
        scanf("%s", guess);

        if (strcasecmp(guess, "SHOW") == 0) {
            printf("Player 2's answer is: %s\n", player2Answer);
            return; // Exit the game and return to the main menu
        }

        if (!VALID_INPUT(guess, 4)) {
            printf("Invalid input. Try again.\n");
            continue;
        }
        evaluateGuess(player2Answer, guess, &A, &B);
        attempts2++;

        if (A == 4) {
            printf("Player 1 guessed correctly in %d attempts!\n", attempts2);
            break;
        } else {
            printf("Result: %dA%dB\n", A, B);
        }
    }

    // Step 5: Compare results and declare the winner
    printf("\nGame Over!\n");
    printf("Player 1 attempts: %d\n", attempts2);
    printf("Player 2 attempts: %d\n", attempts1);

    if (attempts1 < attempts2) printf("Player 2 wins!\n");
    else if (attempts1 > attempts2) printf("Player 1 wins!\n");
    else printf("It's a tie!\n");
}

// Function to display overall game statistics
void displayStats(const Stats *stats) {
    PRINT_LINE();
    printf("Games Played: %d\n", stats->totalGames);
    printf("Average Rounds: %.2f\n", stats->totalGames > 0 ?
           (double)stats->totalRounds / stats->totalGames : 0.0);
    printf("Best Game: %d rounds\n", stats->bestGame);
}


