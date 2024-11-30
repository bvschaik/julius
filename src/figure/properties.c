#include "properties.h"

static const figure_properties properties[FIGURE_TYPE_MAX] = {
    [FIGURE_NONE] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_IMMIGRANT] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_EMIGRANT] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_HOMELESS] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CART_PUSHER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_LABOR_SEEKER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_EXPLOSION] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TAX_COLLECTOR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENGINEER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WAREHOUSEMAN] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_PREFECT] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 50, .attack_value = 5, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FORT_JAVELIN] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 80, .attack_value = 4, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 4, .missile_delay = 100
    },
    [FIGURE_FORT_MOUNTED] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 120, .attack_value = 8, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FORT_LEGIONARY] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 150, .attack_value = 10, .defense_value = 2,
    .missile_defense_value = 0, .missile_attack_value = 2, .missile_delay = 150
    },
    [FIGURE_FORT_STANDARD] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ACTOR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_GLADIATOR] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 100, .attack_value = 9, .defense_value = 2,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_LION_TAMER] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 100, .attack_value = 15, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CHARIOTEER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TRADE_CARAVAN] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TRADE_SHIP] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TRADE_CARAVAN_DONKEY] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_PROTESTER] = {
    .category = FIGURE_CATEGORY_CRIMINAL,
    .max_damage = 12, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CRIMINAL] = {
    .category = FIGURE_CATEGORY_CRIMINAL,
    .max_damage = 12, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_RIOTER] = {
    .category = FIGURE_CATEGORY_CRIMINAL,
    .max_damage = 12, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FISHING_BOAT] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MARKET_TRADER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_PRIEST] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_SCHOOL_CHILD] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TEACHER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_LIBRARIAN] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_BARBER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_BATHHOUSE_WORKER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_DOCTOR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_SURGEON] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WORKER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MAP_FLAG] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FLOTSAM] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 0, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_DOCKER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MARKET_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_PATRICIAN] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_INDIGENOUS_NATIVE] = {
    .category = FIGURE_CATEGORY_NATIVE,
    .max_damage = 40, .attack_value = 6, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TOWER_SENTRY] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 50, .attack_value = 6, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 6, .missile_delay = 50
    },
    [FIGURE_ENEMY43_SPEAR] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 70, .attack_value = 5, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 4, .missile_delay = 70
    },
    [FIGURE_ENEMY44_SWORD] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 90, .attack_value = 7, .defense_value = 1,
    .missile_defense_value = 1, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY45_SWORD] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 120, .attack_value = 12, .defense_value = 2,
    .missile_defense_value = 2, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY46_CAMEL] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 120, .attack_value = 7, .defense_value = 1,
    .missile_defense_value = 0, .missile_attack_value = 5, .missile_delay = 70
    },
    [FIGURE_ENEMY47_ELEPHANT] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 200, .attack_value = 20, .defense_value = 5,
    .missile_defense_value = 8, .missile_attack_value = 6, .missile_delay = 70
    },
    [FIGURE_ENEMY48_CHARIOT] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 120, .attack_value = 15, .defense_value = 4,
    .missile_defense_value = 4, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY49_FAST_SWORD] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 90, .attack_value = 7, .defense_value = 1,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY50_SWORD] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 110, .attack_value = 10, .defense_value = 2,
    .missile_defense_value = 2, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY51_SPEAR] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 70, .attack_value = 5, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 3, .missile_delay = 100
    },
    [FIGURE_ENEMY52_MOUNTED_ARCHER] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 100, .attack_value = 6, .defense_value = 1,
    .missile_defense_value = 0, .missile_attack_value = 4, .missile_delay = 70
    },
    [FIGURE_ENEMY53_AXE] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 120, .attack_value = 15, .defense_value = 2,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY54_GLADIATOR] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 100, .attack_value = 9, .defense_value = 2,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY_CAESAR_JAVELIN] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 90, .attack_value = 4, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 4, .missile_delay = 100
    },
    [FIGURE_ENEMY_CAESAR_MOUNTED] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 100, .attack_value = 8, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ENEMY_CAESAR_LEGIONARY] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 150, .attack_value = 13, .defense_value = 2,
    .missile_defense_value = 2, .missile_attack_value = 2, .missile_delay = 150
    },
    [FIGURE_NATIVE_TRADER] = {
    .category = FIGURE_CATEGORY_NATIVE,
    .max_damage = 40, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ARROW] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 12, .missile_delay = 0
    },
    [FIGURE_JAVELIN] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 20, .missile_delay = 0
    },
    [FIGURE_BOLT] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 200, .missile_delay = 0
    },
    [FIGURE_BALLISTA] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 200
    },
    [FIGURE_CREATURE] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MISSIONARY] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FISH_GULLS] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_DELIVERY_BOY] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_SHIPWRECK] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_SHEEP] = {
    .category = FIGURE_CATEGORY_ANIMAL,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WOLF] = {
    .category = FIGURE_CATEGORY_AGGRESSIVE_ANIMAL,
    .max_damage = 80, .attack_value = 8, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_ZEBRA] = {
    .category = FIGURE_CATEGORY_ANIMAL,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_SPEAR] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 10, .missile_delay = 0
    },
    [FIGURE_HIPPODROME_HORSES] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WORK_CAMP_WORKER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WORK_CAMP_SLAVE] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WORK_CAMP_ARCHITECT] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MESS_HALL_SUPPLIER] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 70, .attack_value = 8, .defense_value = 1,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MESS_HALL_COLLECTOR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_PRIEST_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_BARKEEP] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_BARKEEP_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_TOURIST] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_WATCHMAN] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 50, .attack_value = 6, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 6, .missile_delay = 50
    },
    [FIGURE_WATCHTOWER_ARCHER] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 40
    },
    [FIGURE_FRIENDLY_ARROW] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 10, .missile_delay = 0
    },
    [FIGURE_CARAVANSERAI_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CRIMINAL_ROBBER] = {
    .category = FIGURE_CATEGORY_CRIMINAL,
    .max_damage = 12, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CRIMINAL_LOOTER] = {
    .category = FIGURE_CATEGORY_CRIMINAL,
    .max_damage = 12, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_CARAVANSERAI_COLLECTOR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_LIGHTHOUSE_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_MESS_HALL_FORT_SUPPLIER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_DEPOT_CART_PUSHER] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 20, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FORT_INFANTRY] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 110, .attack_value = 8, .defense_value = 1,
    .missile_defense_value = 2, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_BEGGAR] = {
    .category = FIGURE_CATEGORY_CITIZEN,
    .max_damage = 10, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 0, .missile_delay = 0
    },
    [FIGURE_FORT_ARCHER] = {
    .category = FIGURE_CATEGORY_ARMED,
    .max_damage = 90, .attack_value = 6, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 4, .missile_delay = 50
    },
    [FIGURE_ENEMY_CATAPULT] = {
    .category = FIGURE_CATEGORY_HOSTILE,
    .max_damage = 200, .attack_value = 1, .defense_value = 0,
    .missile_defense_value = 20, .missile_attack_value = 100, .missile_delay = 200
    },
    [FIGURE_CATAPULT_MISSILE] = {
    .category = FIGURE_CATEGORY_INACTIVE,
    .max_damage = 100, .attack_value = 0, .defense_value = 0,
    .missile_defense_value = 0, .missile_attack_value = 200, .missile_delay = 0
    },

};

const figure_properties *figure_properties_for_type(figure_type type)
{
    return &properties[type];
}
