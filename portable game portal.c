#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

// ------------------------------------------------------------
// Memory structure for libcurl
// ------------------------------------------------------------
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// ------------------------------------------------------------
// Download CSV from public Google Sheets link
// ------------------------------------------------------------
char *download_csv(const char *url) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            chunk.memory = NULL;
        }
        curl_easy_cleanup(curl);
    }
    return chunk.memory; // caller must free
}

// ------------------------------------------------------------
// Find player level from CSV data (assumes first column = name, second = level)
// ------------------------------------------------------------
int find_player_level(const char *csv_data, const char *player_name) {
    char *data_copy = strdup(csv_data);
    char *line = strtok(data_copy, "\n");
    while (line) {
        char name[100];
        int level;
        // Adjust this sscanf format to match your CSV columns!
        // Currently assumes: Name, Level
        if (sscanf(line, "%[^,],%d", name, &level) == 2) {
            if (strcmp(name, player_name) == 0) {
                free(data_copy);
                return level;
            }
        }
        line = strtok(NULL, "\n");
    }
    free(data_copy);
    return -1;
}

// ------------------------------------------------------------
// Game formulas – adjust these based on your spreadsheet and desired balance
// ------------------------------------------------------------
int player_damage(int level) {
    return level * 5;           // Example: level 10 → 50 damage
}

int player_max_health(int level) {
    return 100 + level * 2;     // Player gets a bit tougher with level (optional)
}

int npc_health(int stage, int player_level) {
    int base = 30 + stage * 10; // Base health increases with stage
    int reduction = player_level * 2; // Higher player level reduces enemy health
    int result = base - reduction;
    return (result < 5) ? 5 : result; // Minimum 5 health
}

int npc_damage(int stage, int player_level) {
    int base = 5 + stage * 2;   // Base damage increases with stage
    int reduction = player_level; // Higher player level reduces enemy damage
    int result = base - reduction;
    return (result < 1) ? 1 : result; // Minimum 1 damage
}

// ------------------------------------------------------------
// Main game
// ------------------------------------------------------------
int main() {
    const char *csv_url = "https://docs.google.com/spreadsheets/d/e/2PACX-1vR1fIAh8MdZcuNzAY7vpyj5absQe135wJxIWMu1W-dEWOxHShn3NLNKnZgFTMiASfUTfqBH1fC6JV5N/pub?output=csv";

    printf("=== SIMPLE SHOOTER (Live Data) ===\n");
    printf("Fetching player data from Google Sheets...\n");

    char *csv_data = download_csv(csv_url);
    if (!csv_data) {
        printf("Failed to download data. Check URL and internet.\n");
        return 1;
    }

    char player_name[100];
    printf("Enter your name: ");
    fgets(player_name, sizeof(player_name), stdin);
    player_name[strcspn(player_name, "\n")] = 0; // trim newline

    int player_level = find_player_level(csv_data, player_name);
    free(csv_data);

    if (player_level == -1) {
        printf("Player '%s' not found in the spreadsheet.\n", player_name);
        return 1;
    }

    if (player_level > 100) player_level = 100; // Cap at 100

    printf("\nWelcome, %s! Your level is %d.\n", player_name, player_level);
    printf("Your damage: %d\n", player_damage(player_level));
    printf("Your health: %d\n", player_max_health(player_level));
    printf("Enemies get stronger each stage, but your high level weakens them!\n");
    printf("\nPress Enter to start...");
    getchar();

    int stage = 1;
    int player_hp = player_max_health(player_level);

    while (1) {
        // Create enemy for this stage
        int enemy_hp = npc_health(stage, player_level);
        int enemy_dmg = npc_damage(stage, player_level);

        printf("\n=== STAGE %d ===\n", stage);
        printf("Enemy: %d HP, %d DMG\n", enemy_hp, enemy_dmg);
        printf("Your HP: %d\n", player_hp);

        // Fight this enemy
        while (enemy_hp > 0 && player_hp > 0) {
            printf("\nAction? (a=attack / r=run): ");
            char action;
            scanf(" %c", &action);

            if (action == 'a') {
                // Player attacks
                int dmg = player_damage(player_level);
                enemy_hp -= dmg;
                printf("You attack! Dealt %d damage. Enemy HP: %d\n", dmg, (enemy_hp > 0 ? enemy_hp : 0));

                if (enemy_hp <= 0) {
                    printf("You defeated the enemy!\n");
                    break;
                }

                // Enemy counterattacks
                player_hp -= enemy_dmg;
                printf("Enemy hits you for %d damage. Your HP: %d\n", enemy_dmg, (player_hp > 0 ? player_hp : 0));

                if (player_hp <= 0) {
                    printf("You have been defeated... Game Over.\n");
                    return 0;
                }
            }
            else if (action == 'r') {
                printf("You ran away. Game over.\n");
                return 0;
            }
            else {
                printf("Invalid action. Use 'a' to attack, 'r' to run.\n");
            }
        }

        // Victory – proceed to next stage
        stage++;
        // Optional: heal player a bit between stages?
        // player_hp = player_max_health(player_level); // uncomment to fully heal each stage
        printf("\nPrepare for stage %d!\n", stage);
    }

    return 0;
}
