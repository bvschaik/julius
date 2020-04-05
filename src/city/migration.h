#ifndef CITY_MIGRATION_H
#define CITY_MIGRATION_H

void city_migration_update(void);

void city_migration_determine_no_immigration_cause(void);

int city_migration_no_immigration_cause(void);

int city_migration_no_room_for_immigrants(void);

int city_migration_percentage(void);

int city_migration_newcomers(void);

void city_migration_reset_newcomers(void);

#endif // CITY_MIGRATION_H
