#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"

#define PLAYER_WALL_MAX 5

#define WHEEL_BRONZE_SIDES 11

#define HERO_XP_MAX 6
#define HERO_TIERS_COUNT 3

#define BITS_SET(N, M) (((N) & (M)) == (M))

enum hero_type_id {
    hero_type_knight,
    hero_type_archer,
    hero_type_mage,
    hero_type_count
};
enum hero_type_id hero_attack_order[hero_type_count] = {hero_type_knight, hero_type_archer,
                                                        hero_type_mage};
struct hero_type;
struct hero {
    struct hero_type *type;
    int energy;
    int xp;
    int tier;
};

#define SIDE_XP_MASK (1 << 2)
#define SIDE_DIAMOND_MASK (1 << 3)
#define SIDE_DIAMOND_VALUE_MASK 3
#define SIDE_SQUARE_MASK (1 << 4)
#define SIDE_SQUARE_VALUE_MASK 3
#define SIDE_HAMMER_MASK (1 << 5)
#define SIDE_HAMMER_VALUE_MASK 3

// TODO: use union or something
enum wheel_side {
    wheel_side_empty = 0,
    diamond1 = SIDE_DIAMOND_MASK + 1,
    diamond2 = SIDE_DIAMOND_MASK + 2,
    diamond1xp = diamond1 + SIDE_XP_MASK,
    diamond2xp = diamond2 + SIDE_XP_MASK,
    square1 = SIDE_SQUARE_MASK + 1,
    square2 = SIDE_SQUARE_MASK + 2,
    square1xp = square1 + SIDE_XP_MASK,
    square2xp = square2 + SIDE_XP_MASK,
    hammer1 = SIDE_HAMMER_MASK + 1,
    hammer2 = SIDE_HAMMER_MASK + 2
};
enum wheel_side bronze_wheel[11] = {wheel_side_empty, diamond1, diamond2, diamond1xp, diamond2xp,
                                    square1,
                                    square2,
                                    square1xp, square2xp, hammer1, hammer2};

// TODO: Either move everything into game struct or global variables
struct wheels_game {
    int player_turn;
} game;

struct player {
    int hp;
    int wall;
    struct hero hero1;
    struct hero hero2;
    int wheel[5];
} player1, player2;

struct hero_type {
    enum hero_type_id id;
    char *name;
    char *description;
    int maxEnergy[HERO_TIERS_COUNT];
    int damageCrown[HERO_TIERS_COUNT];
    int damageWall[HERO_TIERS_COUNT];

    void (*attack)(struct hero *hero, struct player *attacker, struct player *defender);
};

char *hero_type_getName(struct hero_type *type) {
    return type->name;
}

int hero_getDamageWall(struct hero *hero) {
    return hero->type->damageWall[hero->tier];
}

int hero_getDamageCrown(struct hero *hero) {
    return hero->type->damageCrown[hero->tier];
}

int hero_getMaxEnergy(struct hero *hero) {
    return hero->type->maxEnergy[hero->tier];
}

void hero_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    hero->type->attack(hero, attacker, defender);
}

struct hero_type hero_types[hero_type_count];

void knight_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    if (defender->wall > 0) {
        defender->wall -= hero_getDamageWall(hero);
    } else defender->hp -= hero_getDamageCrown(hero);
}

void archer_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    if (defender->wall > 2) {
        defender->wall -= hero_getDamageWall(hero);
    } else defender->hp -= hero_getDamageCrown(hero);
}

void mage_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    if (defender->wall > 0) {
        defender->wall -= hero_getDamageWall(hero);
    } else defender->hp -= hero_getDamageCrown(hero);
    // Double attack
    defender->hp -= hero_getDamageCrown(hero);
}

/**
 * Has to be called before the game can work
 */
void wheels_init() {
    // For RNG
    srand(time(NULL));
    hero_types[hero_type_knight] = (struct hero_type) {
            .id = hero_type_knight,
            .name = "Knight",
            .description = "If the opponent has a Wall, attacks the Wall. Otherwise attacks the Crown.",
            .maxEnergy = {3, 3, 3},
            .damageCrown = {3, 5, 7},
            .damageWall = {3, 5, 5},
            .attack = knight_attack
    };
    hero_types[hero_type_archer] = (struct hero_type) {
            .id = hero_type_archer,
            .name = "Archer",
            .description = "Can shoot over Walls that are at most 2 high.",
            .maxEnergy = {4, 3, 3},
            .damageCrown = {3, 4, 6},
            .damageWall = {1, 2, 3},
            .attack = archer_attack
    };
    hero_types[hero_type_mage] = (struct hero_type) {
            .id = hero_type_mage,
            .name = "Mage",
            .description = "Attacks twice. Second attack ignores Walls.",
            .maxEnergy = {5, 4, 4},
            .damageCrown = {3, 3, 3},
            .damageWall = {1, 3, 5},
            .attack = mage_attack
    };
}

void initHero(struct hero *hero) {
    hero->energy = hero->type->maxEnergy[0];
    hero->tier = 0;
    hero->xp = 0;
}

void reset() {
    player1.hp = 10;
    player1.wall = 0;
    // player1.wheel should be all zero

    // Copy player 1
    player2 = player1;
}

int getWheelSideValue(int slot) {
    return slot & ((1 << 30) - 1);
}

void formatWheelLock(char buf[14], const int wheel[5]) {
    for (int i = 0; i < 5; i++) {
        char *bufIndex = buf + 3 * i;
        if (wheel[i] & (1 << 30)) {
            memset(bufIndex, 'X', 2);
        }
    }
}

void formatWheelSlots(char buf[14], int wheel[5]) {
    for (int i = 0; i < 5; i++) {
        char *slot = buf + 3 * i;
        int side = getWheelSideValue(wheel[i]);
        if (side == wheel_side_empty) {
            memcpy(slot, "--", 2);
        } else if (BITS_SET(side, SIDE_DIAMOND_MASK)) {
            slot[0] = '0' + (side & 3);
            if (BITS_SET(side, SIDE_XP_MASK)) {
                slot[1] = 'R';
            } else slot[1] = 'r';
        } else if (BITS_SET(side, SIDE_SQUARE_MASK)) {
            slot[0] = '0' + (side & 3);
            if (BITS_SET(side, SIDE_XP_MASK)) {
                slot[1] = 'L';
            } else slot[1] = 'l';
        } else if (BITS_SET(side, SIDE_HAMMER_MASK)) {
            slot[0] = '0' + (side & 3);
            slot[1] = 'W';
        }
    }
}

// TODO: Hero names
void printStatus() {
    char centerStrBuf[15];
    memset(centerStrBuf, ' ', 14);
    centerStrBuf[14] = '\0';
    // player 2 first
    printf("                     Player 2");
    if (game.player_turn == 2)
        printf("*");
    printf("\n");
    printf("  ");
    printSpaceL(player2.hero1.type->name, 7);
    printf("         ");
    formatWheelSlots(centerStrBuf, player2.wheel);
    printf("%s", centerStrBuf);
    printf("         %s\n", player2.hero2.type->name);

    printf("   Energy %d/%d     ", player2.hero1.energy, hero_getMaxEnergy(&player2.hero1));
    memset(centerStrBuf, ' ', 14);
    formatWheelLock(centerStrBuf, player2.wheel);
    printf("%s", centerStrBuf);
    printf("     %d/%d Energy\n", player2.hero2.energy, hero_getMaxEnergy(&player2.hero2));

    printf("     Tier %d/%d     Health      %02d     %d/%d Tier\n", player2.hero1.tier + 1,
           HERO_TIERS_COUNT, player2.hp, player2.hero2.tier + 1, HERO_TIERS_COUNT);

    printf("       XP %d/%d                        %d/%d XP\n", player2.hero1.xp, HERO_XP_MAX,
           player2.hero2.xp, HERO_XP_MAX);

    printf("    Crown  %d      Wall         %d      %d  Crown\n",
           hero_getDamageCrown(&player2.hero1),
           player2.wall, hero_getDamageCrown(&player2.hero2));

    printf("     Wall  %d                          %d  Wall\n", hero_getDamageWall(&player2.hero1),
           hero_getDamageWall(&player2.hero2));
    printf("\n");
    // Player 1
    printf("     Wall  %d                          %d  Wall\n", hero_getDamageWall(&player1.hero1),
           hero_getDamageWall(&player1.hero2));
    printf("    Crown  %d      Wall         %d      %d  Crown\n",
           hero_getDamageCrown(&player1.hero1),
           player1.wall, hero_getDamageCrown(&player1.hero2));
    printf("       XP %d/%d                        %d/%d XP\n", player1.hero1.xp, HERO_XP_MAX,
           player1.hero2.xp, HERO_XP_MAX);
    printf("     Tier %d/%d     Health      %02d     %d/%d Tier\n", player1.hero1.tier,
           HERO_TIERS_COUNT, player1.hp, player1.hero2.tier, HERO_TIERS_COUNT);
    printf("   Energy %d/%d     ", player1.hero1.energy, hero_getMaxEnergy(&player1.hero1));
    memset(centerStrBuf, ' ', 14);
    formatWheelLock(centerStrBuf, player1.wheel);
    printf("%s", centerStrBuf);
    printf("     %d/%d Energy\n", player1.hero2.energy, hero_getMaxEnergy(&player1.hero2));
    printf("  ");
    printSpaceL(player1.hero1.type->name, 7);
    printf("         ");
    memset(centerStrBuf, ' ', 14);
    formatWheelSlots(centerStrBuf, player1.wheel);
    printf("%s", centerStrBuf);
    printf("         %s\n", player1.hero2.type->name);
    printf("                     Player 1");
    if (game.player_turn == 1)
        printf("*");
    printf("\n");
}

int isWheelFullyLocked(const int wheel[5]) {
    for (int i = 0; i < 5; i++) {
        if (!(wheel[i] & (1 << 30)))
            return 0;
    }
    return 1;
}

int handlePlayerTurn(int wheel[5]) {
    int turnsLeft = 3;
    do {
        printStatus();
        printf("> ");
        int input = getchar();
        printf("\n");
        printf("%d\n", getchar());
        // Check forfeit
        if (input == 'f') {
            return 1;
        }
        // Player wants to (un)lock wheel; They can't do so on the first turn
        if (turnsLeft < 3 && input <= '5' && input > '0') {
            // Toggle wheel lock
            wheel[(input - '1')] ^= 1 << 30;
        } else if (input == 'e') {
            // handle fully locked wheel
            if (isWheelFullyLocked(wheel)) {
                break;
            } else {
                // Spin wheel
                for (int i = 0; i < 5; i++) {
                    // If locked, skip slot
                    if (wheel[i] & (1 << 30))
                        continue;
                    int side = rand() % WHEEL_BRONZE_SIDES;
                    // Reset the slot first and then set the side
                    wheel[i] = (wheel[i] & ~((1 << 30) - 1)) | bronze_wheel[side];
                }
                turnsLeft--;
            }
        }
    } while (turnsLeft > 0);
    // Unlock wheel
    for (int i = 0; i < 5; i++) {
        wheel[i] &= ~(1 << 30);
    }
    return 0;
}

// TODO: check for max energy and decrease if needed
void updateHeroTier(struct hero *hero) {
    if (hero->xp >= HERO_XP_MAX && hero->tier < 3) {
        hero->tier++;
        hero->xp = 0;
    }
}

void updateHeroTiers(struct player *player) {
    updateHeroTier(&player->hero1);
    updateHeroTier(&player->hero2);
}

void assignWheelXp(struct player *player) {
    int diamondXp = 0, squareXp = 0;
    for (size_t i = 0; i < 5; i++) {
        int side = getWheelSideValue(player->wheel[i]);
        if (!BITS_SET(side, SIDE_XP_MASK))
            continue;
        if (BITS_SET(side, SIDE_DIAMOND_MASK))
            diamondXp++;
        else if (BITS_SET(side, SIDE_SQUARE_MASK))
            squareXp++;
    }
    player->hero1.xp += squareXp;
    player->hero2.xp += diamondXp;
}

void buildWall(struct player *player) {
    int hammers = 0;
    for (size_t i = 0; i < 5; i++) {
        int side = getWheelSideValue(player->wheel[i]);
        if (BITS_SET(side, SIDE_HAMMER_MASK))
            hammers += side & SIDE_HAMMER_VALUE_MASK;
    }
    if (hammers > 2)
        player->wall += hammers - 2;
    if (player->wall > PLAYER_WALL_MAX)
        player->wall = PLAYER_WALL_MAX;
}

void assignHeroEnergy(struct player *player) {
    int diamondEnergy = 0, squareEnergy = 0;
    for (size_t i = 0; i < 5; i++) {
        int side = getWheelSideValue(player->wheel[i]);
        if (BITS_SET(side, SIDE_DIAMOND_MASK))
            diamondEnergy += side & SIDE_DIAMOND_VALUE_MASK;
        else if (BITS_SET(side, SIDE_SQUARE_MASK))
            squareEnergy += side & SIDE_SQUARE_VALUE_MASK;
    }
    if (squareEnergy > 2)
        player->hero1.energy -= (squareEnergy - 2);
    if (diamondEnergy > 2)
        player->hero2.energy -= (diamondEnergy - 2);
}

void tryHeroAttack(struct hero *hero, struct player *attacker, struct player *defender,
                   enum hero_type_id typeId) {
    if (hero->type->id != typeId)
        return;
    // Regular attack
    if (hero->energy <= 0) {
        hero_attack(hero, attacker, defender);
        hero->energy = hero_getMaxEnergy(hero);
        hero->xp += 2;
    }
    // Bomb attack
    if (hero->xp >= HERO_XP_MAX && hero->tier == HERO_TIERS_COUNT) {
        defender->hp -= 2;
        hero->xp = 0;
    }
}

void fixPlayerStats(struct player *player) {
    if (player->hp < 0)
        player->hp = 0;
    if (player->wall < 0)
        player->wall = 0;
}

void handleAttacks() {
    for (size_t i = 0; i < hero_type_count; i++) {
        enum hero_type_id typeId = hero_attack_order[i];
        tryHeroAttack(&player1.hero1, &player1, &player2, typeId);
        tryHeroAttack(&player1.hero2, &player1, &player2, typeId);
        tryHeroAttack(&player2.hero1, &player2, &player1, typeId);
        tryHeroAttack(&player2.hero2, &player2, &player1, typeId);
    }

    // Fix stats
    fixPlayerStats(&player1);
    fixPlayerStats(&player2);
}

void playRound() {
    // assign XP
    assignWheelXp(&player1);
    assignWheelXp(&player2);
    // Update Tiers
    updateHeroTiers(&player1);
    updateHeroTiers(&player2);
    // build wall
    buildWall(&player1);
    buildWall(&player2);
    // assign energy
    assignHeroEnergy(&player1);
    assignHeroEnergy(&player2);
    // attack: knight, archer + bomb attack
    handleAttacks();
    // Update Tiers again
    updateHeroTiers(&player1);
    updateHeroTiers(&player2);
}

int runGameLoop() {
    int player_won = 0;
    do {
        // player 1
        game.player_turn = 1;
        int res = handlePlayerTurn(player1.wheel);
        if (res == 1) {
            // Forfeit
            player_won = 2;
            break;
        }
        // player 2
        game.player_turn = 2;
        res = handlePlayerTurn(player2.wheel);
        if (res == 1) {
            // Forfeit
            player_won = 1;
            break;
        }
        playRound();
        printStatus();
        // check game end
        if (player1.hp == 0 && player2.hp == 0) {
            player_won = -1;
        } else if (player1.hp == 0) {
            player_won = 1;
        } else if (player2.hp == 0) {
            player_won = 2;
        }
    } while (!player_won);
    // Win screen
    if (player_won == 1) {
        printf("Player 1 wins!\n");
    } else if (player_won == 2) {
        printf("Player 2 wins!\n");
    } else {
        printf("Tie!\n");
    }
    return 0;
}

int selectHero(int excludeId) {
    do {
        for (int i = 0; i < hero_type_count; i++) {
            if (excludeId != hero_types[i].id)
                printf("%d ", hero_types[i].id);
        }
        printf("\nSelect a hero\n>");
        char choice = getchar();
        getchar();
        printf("\n");
        int selection = choice - '0';
        if (selection < 0 || selection >= hero_type_count) {
            printf("That hero does not exist.\n");
        } else if (selection == excludeId) {
            printf("You cannot select that hero.\n");
        } else {
            return selection;
        }
    } while (1);
}

void hero_type_printInfo(struct hero_type *type) {
    printf("%s\n", hero_type_getName(type)); // Name
    // Tiers
    printf("Tier        ");
    for (int i = 0; i < HERO_TIERS_COUNT; i++) {
        printf("  %d", i + 1);
    }
    printf("\n");
    printSpaceR("Max Energy", 14);
    printIntArray(HERO_TIERS_COUNT, type->maxEnergy);
    printf("\n");
    printSpaceR("Crown Damage", 14);
    printIntArray(HERO_TIERS_COUNT, type->damageCrown);
    printf("\n");
    printSpaceR("Wall Damage", 14);
    printIntArray(HERO_TIERS_COUNT, type->damageWall);
    printf("\n");
    printSpaceR("Description", 14);
    printf("%s\n", type->description);
}

// See doc/heroSelect.txt
void handleHeroSelection(struct player *player) {
    int confirm1 = 0, confirm2 = 0;
    do {
        for (int i = 0; i < hero_type_count; i++) {
            char buf[20];
            snprintf(buf, 20, "[%d] %s", i, hero_type_getName(hero_types + i));
            printSpaceR(buf, 14);
            if (player->hero1.type == hero_types + i) {
                printf("< 1");
            } else if (player->hero2.type == hero_types + i) {
                printf("< 2");
            }
            printf("\n");
        }
        // Print hero type stats
        if (confirm1 && player->hero2.type != NULL) {
            printf("\n");
            hero_type_printInfo(player->hero2.type);
        } else if (player->hero1.type != NULL) {
            printf("\n");
            hero_type_printInfo(player->hero1.type);
        }
        // Prompt
        if (player->hero1.type == NULL) {
            printf("\nSelect your first Hero [0-%d]\n>", hero_type_count - 1);
        } else if (confirm1 && player->hero2.type == NULL) {
            printf("\nSelect your second Hero [0-%d]\n>", hero_type_count - 1);
        } else {
            printf("\nEnter [y]es to confirm or select another Hero [0-2]\n>");
        }
        // Handle input
        char input = getchar();
        consumeLine();
        short inputYes = (input == 'y' || input == 'Y');
        // When second Hero is selected, player may confirm 
        if (player->hero2.type != NULL && inputYes) {
            confirm2 = 1;
        } else if (!confirm1 && player->hero1.type != NULL && inputYes) {
            // When first Hero is selected, but not yet confirmed, player may confirm
            confirm1 = 1;
        } else {
            // Otherwise select
            int numChoice = input - '0';
            if (numChoice < 0 || numChoice >= hero_type_count) {
                printf("That Hero does not exist\n");
            } else if (confirm1) {
                if (player->hero1.type == hero_types + numChoice) {
                    printf("You have already selected %s\n",
                           hero_type_getName(hero_types + numChoice));
                } else
                    player->hero2.type = hero_types + numChoice;
            } else
                player->hero1.type = hero_types + numChoice;
        }
        printf("\n");
    } while (!confirm1 || !confirm2);
}

int main() {
    setbuf(stdout, 0);
    wheels_init();
    reset();
    printf("Player 1 Hero Selection\n");
    handleHeroSelection(&player1);
    printf("Player 2 Hero Selection\n");
    handleHeroSelection(&player2);
    return runGameLoop();
}
