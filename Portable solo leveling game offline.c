#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Adjust these formulas based on your spreadsheet data
int player_damage(int level) {
    return level * 4;          // Example: level 1 → 4 damage
}

int npc_health(int level) {
    return 40 - level * 2;      // Gets lower as player levels up (min 5)
}

int npc_damage(int level) {
    int dmg = 6 - level;        // Gets lower, minimum 1
    return (dmg < 1) ? 1 : dmg;
}

int main() {
    char name[50];
    int level;
    int npc_count = 1;

    printf("=========================\n");
    printf("   SIMPLE SHOOTER GAME\n");
    printf("=========================\n");
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    printf("Enter your current level (1-10): ");
    scanf("%d", &level);

    if (level < 1) level = 1;
    if (level > 10) level = 10;

    printf("\nWelcome, %s! You are level %d.\n", name, level);
    printf("Your damage is %d.\n\n", player_damage(level));
    printf("Press Enter to start...");
    getchar(); getchar();  // wait for key press

    srand(time(NULL));     // seed random for variety (optional)

    while (1) {
        int npc_hp = npc_health(level);
        int npc_dmg = npc_damage(level);
        char npc_name[20];
        sprintf(npc_name, "Enemy #%d", npc_count);

        printf("\n>>> %s appears! <<<\n", npc_name);
        printf("%s has %d health and deals %d damage.\n", npc_name, npc_hp, npc_dmg);

        while (npc_hp > 0) {
            char action;
            printf("\nAction? (a=attack / r=run): ");
            scanf(" %c", &action);

            if (action == 'a') {
                npc_hp -= player_damage(level);
                printf("You shoot! You deal %d damage.\n", player_damage(level));

                if (npc_hp <= 0) {
                    printf("You defeated %s!\n", npc_name);
                    level++;
                    npc_count++;
                    printf("*** Level up! You are now level %d (damage %d) ***\n",
                           level, player_damage(level));
                    break;
                }

                // enemy counterattack (simplified – you don't have health, but we can pretend)
                printf("%s fires back! You take %d damage (ignored in this demo).\n",
                       npc_name, npc_dmg);
            }
            else if (action == 'r') {
                printf("You retreat. Game over.\n");
                return 0;
            }
            else {
                printf("Invalid action.\n");
            }
        }

        // After 5 enemies, ask to continue
        if (npc_count > 5) {
            char again;
            printf("\nYou've beaten 5 enemies. Play another round? (y/n): ");
            scanf(" %c", &again);
            if (again != 'y' && again != 'Y') {
                break;
            }
            npc_count = 1;  // reset enemy counter
        }
    }

    printf("\nThanks for playing!\n");
    return 0;
}
