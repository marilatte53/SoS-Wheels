#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define PLAYER_WALL_MAX 5

#define WHEEL_BRONZE_SIDES 11

#define HERO_XP_MAX 6
#define HERO_TIER_MAX 3

#define BITS_SET(N, M) (((N) & (M)) == (M))

// TODO: move hero base stats to hero type
enum hero_type {
    hero_type_knight,
    hero_type_archer,
    hero_type_count
};
enum hero_type hero_attack_order[] = {hero_type_knight, hero_type_archer};
struct player;

struct hero {
    enum hero_type type; // indicates which hero
    int energy;
    int xp;
    int tier;

    void (*attack)(struct hero *hero, struct player *attacker, struct player *defender);

    // TODO: stats increase with tier
    int maxEnergy;
    int damageCrown;
    int damageWall;
};

#define SIDE_XP_MASK (1 << 2)
#define SIDE_DIAMOND_MASK (1 << 3)
#define SIDE_DIAMOND_VALUE_MASK 3
#define SIDE_SQUARE_MASK (1 << 4)
#define SIDE_SQUARE_VALUE_MASK 3
#define SIDE_HAMMER_MASK (1 << 5)
#define SIDE_HAMMER_VALUE_MASK 3

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

void initHero(struct hero *hero) {
    hero->energy = hero->maxEnergy;
    hero->tier = 1;
    hero->xp = 0;
}

void knight_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    if (defender->wall > 0) {
        defender->wall -= hero->damageWall;
    } else defender->hp -= hero->damageCrown;
}

void initKnight(struct hero *knight) {
    knight->type = hero_type_knight;
    knight->damageWall = 3;
    knight->damageCrown = 3;
    knight->maxEnergy = 3;
    knight->attack = knight_attack;
}

void archer_attack(struct hero *hero, struct player *attacker, struct player *defender) {
    if (defender->wall > 2) {
        defender->wall -= hero->damageWall;
    } else defender->hp -= hero->damageCrown;
}

void initArcher(struct hero *archer) {
    archer->type = hero_type_archer;
    archer->damageWall = 1;
    archer->damageCrown = 3;
    archer->maxEnergy = 4;
    archer->attack = archer_attack;
}

void reset() {
    player1.hp = 10;
    player1.wall = 0;
    // player1.wheel should be all zero

    initKnight(&player1.hero1);
    initArcher(&player1.hero2);
    initHero(&player1.hero1);
    initHero(&player1.hero2);

    // Copy player 1
    player2 = player1;
}

int getWheelSide(int slot) {
    return slot & ((1 << 30) - 1);
}

void formatWheelLock(char buf[14], int wheel[5]) {
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
        int side = getWheelSide(wheel[i]);
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

void printStatus() {
    char centerStrBuf[15];
    memset(centerStrBuf, ' ', 14);
    centerStrBuf[14] = '\0';
    // player 2 first
    printf("                     Player 2");
    if (game.player_turn == 2)
        printf("*");
    printf("\n");
    printf("     Hero  %d      ", player2.hero1.type);
    formatWheelSlots(centerStrBuf, player2.wheel);
    printf("%s", centerStrBuf);
    printf("      %d  Hero\n", player2.hero2.type);

    printf("   Energy %d/%d     ", player2.hero1.energy, player2.hero1.maxEnergy);
    memset(centerStrBuf, ' ', 14);
    formatWheelLock(centerStrBuf, player2.wheel);
    printf("%s", centerStrBuf);
    printf("     %d/%d Energy\n", player2.hero2.energy, player2.hero2.maxEnergy);

    printf("     Tier %d/%d     Health      %02d     %d/%d Tier\n", player2.hero1.tier,
           HERO_TIER_MAX, player2.hp, player2.hero2.tier, HERO_TIER_MAX);

    printf("       XP %d/%d                        %d/%d XP\n", player2.hero1.xp, HERO_XP_MAX,
           player2.hero2.xp, HERO_XP_MAX);

    printf("    Crown  %d      Wall         %d      %d  Crown\n", player2.hero1.damageCrown,
           player2.wall, player2.hero2.damageCrown);

    printf("     Wall  %d                          %d  Wall\n", player2.hero1.damageWall,
           player2.hero2.damageWall);
    printf("\n");
    // Player 1
    printf("     Wall  %d                          %d  Wall\n", player1.hero1.damageWall,
           player1.hero2.damageWall);
    printf("    Crown  %d      Wall         %d      %d  Crown\n", player1.hero1.damageCrown,
           player1.wall, player1.hero2.damageCrown);
    printf("       XP %d/%d                        %d/%d XP\n", player1.hero1.xp, HERO_XP_MAX,
           player1.hero2.xp, HERO_XP_MAX);
    printf("     Tier %d/%d     Health      %02d     %d/%d Tier\n", player1.hero1.tier,
           HERO_TIER_MAX, player1.hp, player1.hero2.tier, HERO_TIER_MAX);
    printf("   Energy %d/%d     ", player1.hero1.energy, player1.hero1.maxEnergy);
    memset(centerStrBuf, ' ', 14);
    formatWheelLock(centerStrBuf, player1.wheel);
    printf("%s", centerStrBuf);
    printf("     %d/%d Energy\n", player1.hero2.energy, player1.hero2.maxEnergy);
    printf("     Hero  %d      ", player1.hero1.type);
    memset(centerStrBuf, ' ', 14);
    formatWheelSlots(centerStrBuf, player1.wheel);
    printf("%s", centerStrBuf);
    printf("      %d  Hero\n", player1.hero2.type);
    printf("                     Player 1");
    if (game.player_turn == 1)
        printf("*");
    printf("\n");

    fflush(stdout);
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
        int side = getWheelSide(player->wheel[i]);
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
        int side = getWheelSide(player->wheel[i]);
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
        int side = getWheelSide(player->wheel[i]);
        if (BITS_SET(side, SIDE_DIAMOND_MASK))
            diamondEnergy += side & SIDE_DIAMOND_VALUE_MASK;
        else if (BITS_SET(side, SIDE_SQUARE_MASK))
            squareEnergy += side & SIDE_SQUARE_VALUE_MASK;
    }
    if(squareEnergy > 2)
        player->hero1.energy -= (squareEnergy - 2);
    if(diamondEnergy > 2)
        player->hero2.energy -= (diamondEnergy - 2);
}

void tryHeroAttack(struct hero *hero, struct player *attacker, struct player *defender, int type) {
    if (hero->type != type)
        return;
    // Regular attack
    if (hero->energy <= 0) {
        hero->attack(hero, attacker, defender);
        hero->energy = hero->maxEnergy;
        hero->xp += 2;
    }
    // Bomb attack
    if (hero->xp >= HERO_XP_MAX && hero->tier == HERO_TIER_MAX) {
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
        enum hero_type type = hero_attack_order[i];
        tryHeroAttack(&player1.hero1, &player1, &player2, type);
        tryHeroAttack(&player1.hero2, &player1, &player2, type);
        tryHeroAttack(&player2.hero1, &player2, &player1, type);
        tryHeroAttack(&player2.hero2, &player2, &player1, type);
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

int main() {
    srand(time(NULL));
    reset();
    return runGameLoop();
}
