#ifndef BUILDING_MAINTENANCE_H
#define BUILDING_MAINTENANCE_H

void building_maintenance_update_fire_direction();
void building_maintenance_update_burning_ruins();
void building_maintenance_check_fire_collapse();
int building_maintenance_get_closest_burning_ruin(int x, int y, int *distance);

#endif // BUILDING_MAINTENANCE_H
