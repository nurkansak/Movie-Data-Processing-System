#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Budget {
    int budget;
    int year;
    char *title;
    char *titletype;
    char *top250;
};

struct Name {
    char **genre;
    char *title;
    char **directors;
    char *mustsee;
    double rating;
    double score;
    char *url;
};

int stringToInt(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            result = result * 10 + (str[i] - '0');
        else
            break;
    }
    return result * sign;
}

char *duplicateString(const char *src) {
    char *newStr;
    if (!src) return NULL;
    newStr = (char *)malloc(strlen(src) + 1);
    if (newStr) strcpy(newStr, src);
    return newStr;
}

char **splitString(const char *input, const char *delimiter, int *count) {
    char *copy;
    char *token;
    int capacity;
    char **result;
    int i; 
    *count = 0;

    copy = duplicateString(input);
    if (!copy) return NULL;

    capacity = 2;
    result = (char **)malloc(capacity * sizeof(char *));
    if (!result) {
        free(copy);
        return NULL;
    }

    token = strtok(copy, delimiter);
    i = 0;
    while (token != NULL) {
        if (*count >= capacity) {
            capacity *= 2;
            result = (char **)realloc(result, capacity * sizeof(char *));
            if (!result) {
                free(copy);
                return NULL;
            }
        }
        result[*count] = duplicateString(token);
        (*count)++;
        token = strtok(NULL, delimiter);
    }
    result = (char **)realloc(result, (*count + 1) * sizeof(char *));
    result[*count] = NULL;

    free(copy);
    return result;
}

void insertBudgetSorted(struct Budget **list, struct Budget newItem, int *count) {
    int i, j; 
    i = 0;
    while (i < *count) {
        if (newItem.year > (*list)[i].year ||
           (newItem.year == (*list)[i].year && newItem.budget > (*list)[i].budget)) {
            break;
        }
        i++;
    }
    *list = (struct Budget *)realloc(*list, (*count + 1) * sizeof(struct Budget));
    j = *count;
    while (j > i) {
        (*list)[j] = (*list)[j - 1];
        j--;
    }
    (*list)[i] = newItem;
    (*count)++;
}

void insertNameSorted(struct Name **list, struct Name newItem, int currentCount, int insertIndex) {
    int j;  
    *list = (struct Name *)realloc(*list, (currentCount + 1) * sizeof(struct Name));
    j = currentCount;
    while (j > insertIndex) {
        (*list)[j] = (*list)[j - 1];
        j--;
    }
    (*list)[insertIndex] = newItem;
}

void readFile(const char *filename, struct Budget **budgetList, struct Name **nameList, int *movieCount) {
    FILE *fp;
    char line[2048];
    char *tokens[20];
    int tokenCount;
    struct Budget b;
    struct Name n;
    int insertIndex;
    int dirCount, genCount;

    fp = fopen(filename, "r");
    if (!fp) {
        printf("File could not be opened: %s\n", filename);
        return;
    }
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *copyLine = duplicateString(line);
        if (!copyLine) break;

        int i; 
        tokenCount = 0;
        tokens[tokenCount] = strtok(copyLine, ";\n");
        while (tokens[tokenCount] != NULL && tokenCount < 20) {
            tokenCount++;
            tokens[tokenCount] = strtok(NULL, ";\n");
        }
        if (tokenCount < 12) {
            free(copyLine);
            continue;
        }

        char *yearStr = tokens[6];
        while (*yearStr == ' ') yearStr++;
        char *endPtr = yearStr + strlen(yearStr) - 1;
        while (endPtr > yearStr && (*endPtr == ' ' || *endPtr == '\n' || *endPtr == '\r')) {
            *endPtr = '\0';
            endPtr--;
        }
        b.year = stringToInt(yearStr);
        if (b.year <= 0) {
            free(copyLine);
            continue;
        }

        b.budget = stringToInt(tokens[0]);
        b.title = duplicateString(tokens[1]);
        b.titletype = duplicateString(tokens[2]);
        b.top250 = duplicateString(tokens[9]);

        n.title = duplicateString(tokens[1]);

        n.directors = splitString(tokens[3], ",", &dirCount);

        n.rating = atof(tokens[4]);
        n.score = atof(tokens[5]);

        n.genre = splitString(tokens[7], ",", &genCount);

        n.mustsee = duplicateString(tokens[10]);
        n.url = duplicateString(tokens[11]);

        insertBudgetSorted(budgetList, b, movieCount);
        insertNameSorted(nameList, n, *movieCount - 1, (*movieCount) - 1);

        free(copyLine);
    }
    fclose(fp);
}

void listBudget(struct Budget *list, int count) {
    int i;  
    for (i = 0; i < count; i++) {
        printf("Budget: %d | Year: %d | Title: %s | Type: %s | Top250: %s\n",
               list[i].budget, list[i].year, list[i].title, list[i].titletype, list[i].top250);
    }
}

void listName(struct Name *list, int count) {
    int i; 
    for (i = 0; i < count; i++) {
        printf("Title: %s | Rating: %.1f | Score: %.1f | URL: %s\n",
               list[i].title, list[i].rating, list[i].score, list[i].url);
    }
}

void listGenres(struct Name *list, int count) {
    char *seen[500];
    int seenCount = 0;
    int i, j, k;  
    for (i = 0; i < count; i++) {
        for (j = 0; list[i].genre[j]; j++) {
            int exists = 0;
            for (k = 0; k < seenCount; k++) {
                if (strcmp(list[i].genre[j], seen[k]) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) {
                seen[seenCount++] = list[i].genre[j];
            }
        }
    }
    printf("Genres:\n");
    for (i = 0; i < seenCount; i++) {
        printf("- %s\n", seen[i]);
    }
}

void listThroughScores(struct Name *list, int count) {
    int i, j;  
    struct Name *sorted = (struct Name *)malloc(count * sizeof(struct Name));
    memcpy(sorted, list, count * sizeof(struct Name));
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (sorted[i].score < sorted[j].score) {
                struct Name tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    for (i = 0; i < count; i++) {
        printf("Title: %s | Score: %.2f\n", sorted[i].title, sorted[i].score);
    }
    free(sorted);
}

void singleMovie(struct Budget *bList, struct Name *nList, int count, const char *title) {
    int i, j; 
    for (i = 0; i < count; i++) {
        if (strcmp(bList[i].title, title) == 0) {
            printf("=== Movie Info ===\n");
            printf("Title: %s\n", bList[i].title);
            printf("Budget: %d | Year: %d | Top250: %s\n", bList[i].budget, bList[i].year, bList[i].top250);
            printf("Type: %s\n", bList[i].titletype);
            printf("Rating: %.1f | Score: %.1f | URL: %s\n", nList[i].rating, nList[i].score, nList[i].url);
            printf("Must See: %s\n", nList[i].mustsee);
            printf("Directors: ");
            for (j = 0; nList[i].directors[j]; j++) {
                printf("%s ", nList[i].directors[j]);
            }
            printf("\nGenres: ");
            for (j = 0; nList[i].genre[j]; j++) {
                printf("%s ", nList[i].genre[j]);
            }
            printf("\n");
            return;
        }
    }
    printf("Movie not found.\n");
}

void genreFrequency(struct Name *list, int count) {
    char *genres[500];
    int freq[500] = {0};
    int gCount = 0;
    int i, j, k;  
    for (i = 0; i < count; i++) {
        for (j = 0; list[i].genre[j]; j++) {
            int found = 0;
            for (k = 0; k < gCount; k++) {
                if (strcmp(list[i].genre[j], genres[k]) == 0) {
                    freq[k]++;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                genres[gCount] = list[i].genre[j];
                freq[gCount] = 1;
                gCount++;
            }
        }
    }
    for (i = 0; i < gCount; i++) {
        printf("%s: %d\n", genres[i], freq[i]);
    }
}

void freeBudgetList(struct Budget *list, int count) {
    int i; 
    for (i = 0; i < count; i++) {
        free(list[i].title);
        free(list[i].titletype);
        free(list[i].top250);
    }
    free(list);
}

void freeNameList(struct Name *list, int count) {
    int i, j;  
    for (i = 0; i < count; i++) {
        free(list[i].title);
        free(list[i].mustsee);
        free(list[i].url);

        for (j = 0; list[i].genre && list[i].genre[j]; j++) {
            free(list[i].genre[j]);
        }
        free(list[i].genre);

        for (j = 0; list[i].directors && list[i].directors[j]; j++) {
            free(list[i].directors[j]);
        }
        free(list[i].directors);
    }
    free(list);
}

int main() {
    struct Budget *budgetList = NULL;
    struct Name *nameList = NULL;
    int movieCount = 0;
    int choice;
    char search[256];

    readFile("movie.txt", &budgetList, &nameList, &movieCount);

    if (movieCount == 0) {
        printf("No movies loaded. Exiting.\n");
        return 0;
    }

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. List of budget array\n");
        printf("2. List of name array\n");
        printf("3. List of genres\n");
        printf("4. List of the Movie Through the Years\n");
        printf("5. List of the Movie Through the Scores\n");
        printf("6. All Information of a Single Movie\n");
        printf("7. Frequency of the Genres\n");
        printf("8. Exit\n");
        printf("Your choice: ");

        char line[100];
        if (!fgets(line, sizeof(line), stdin)) {
            printf("Input error.\n");
            continue;
        }
        if (sscanf(line, "%d", &choice) != 1 || choice < 1 || choice > 8) {
            printf("Invalid input. Please enter a number between 1 and 8.\n");
            continue;
        }

        switch (choice) {
            case 1:
                listBudget(budgetList, movieCount);
                break;
            case 2:
                listName(nameList, movieCount);
                break;
            case 3:
                listGenres(nameList, movieCount);
                break;
            case 4:
                listBudget(budgetList, movieCount);
                break;
            case 5:
                listThroughScores(nameList, movieCount);
                break;
            case 6:
                printf("Enter movie title: ");
                if (fgets(search, sizeof(search), stdin)) {
                    search[strcspn(search, "\n")] = '\0';
                    singleMovie(budgetList, nameList, movieCount, search);
                }
                break;
            case 7:
                genreFrequency(nameList, movieCount);
                break;
            case 8:
                printf("Exiting...\n");
                freeBudgetList(budgetList, movieCount);
                freeNameList(nameList, movieCount);
                return 0;
        }
    }

    return 0;
}
