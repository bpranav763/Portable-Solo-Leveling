#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define WEBAPP_URL "https://script.google.com/macros/s/AKfycbxWwXz4jZsSgoFTO7wGBCsNDKuKuOQmN2rdFgbTDYQVWydPZROdJNHIgACaMIBznTfvjg/exec"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

char *http_get(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        chunk.memory = NULL;
    }

    curl_easy_cleanup(curl);
    return chunk.memory;
}

int get_exp(const char *user) {
    char url[512];
    snprintf(url, sizeof(url), "%s?user=%s&action=get", WEBAPP_URL, user);
    char *response = http_get(url);
    if (!response) return -1;
    int exp = atoi(response);
    free(response);
    return exp;
}

int update_exp(const char *user, int new_exp) {
    char url[512];
    snprintf(url, sizeof(url), "%s?user=%s&action=update&exp=%d", WEBAPP_URL, user, new_exp);

    char *response = http_get(url);
    if (!response) return -1;

    // Check if response is "OK" (trim whitespace)
    int ok = 0;
    char *p = response;
    while (*p == ' ' || *p == '\n' || *p == '\r') p++;
    if (strcmp(p, "OK") == 0) ok = 1;

    free(response);
    return ok ? 0 : -1;
}

int main() {
    char user[100];
    printf("Enter your username (as in the sheet): ");
    if (scanf("%s", user) != 1) {
        printf("Input error.\n");
        return 1;
    }

    int exp = get_exp(user);
    if (exp == -1) {
        printf("Failed to get EXP. Check your URL and internet.\n");
        return 1;
    }
    printf("Your current EXP: %d\n", exp);

    printf("Did you complete the quest (walk 1 km)? (y/n): ");
    char answer;
    int result = scanf(" %c", &answer);
    if (result != 1) {
        printf("Input error.\n");
        return 1;
    }

    int delta = 0;
    if (answer == 'y' || answer == 'Y') {
        delta = 10;
        printf("You gained 10 EXP! Congratulations!\n");
    } else if (answer == 'n' || answer == 'N') {
        delta = -10;
        printf("You lost 10 EXP. Better luck next time!\n");
    } else {
        printf("Invalid input. No changes.\n");
        return 0;
    }

    int new_exp = exp + delta;
    if (update_exp(user, new_exp) == 0) {
        printf("EXP updated. Your new total: %d\n", new_exp);
    } else {
        printf("Failed to update EXP.\n");
    }
    return 0;
}
