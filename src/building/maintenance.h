#ifndef BUILDING_MAINTENANCE_H
#define BUILDING_MAINTENANCE_H

void building_maintenance_update_fire_direction(void);
void building_maintenance_update_burning_ruins(void);
void building_maintenance_check_fire_collapse(void);
int building_maintenance_get_closest_burning_ruin(int x, int y, int *distance);

void building_maintenance_check_rome_access(void);

#endif // BUILDING_MAINTENANCE_H
