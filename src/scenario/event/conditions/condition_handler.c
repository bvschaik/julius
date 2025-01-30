#include "condition_handler.h"

#include "core/log.h"
#include "game/resource.h"
#include "scenario/condition_types/condition_types.h"

static int condition_in_use(const scenario_condition_t *condition)
{
    return condition->type != CONDITION_TYPE_UNDEFINED;
}

void scenario_condition_group_new(scenario_condition_group_t *group, unsigned int id)
{
    // This group has been moved and is not new, no need to setup the array
    if (group->type == FULFILLMENT_TYPE_ANY) {
        return;
    }
    group->type = id == 0 ? FULFILLMENT_TYPE_ALL : FULFILLMENT_TYPE_ANY;
    if (!array_init(group->conditions, CONDITION_GROUP_ITEMS_ARRAY_SIZE_STEP, 0, condition_in_use)) {
        log_error("Unable to allocate enough memory for the scenario condition group. The game will now crash.", 0, 0);
    }
}

int scenario_condition_group_in_use(const scenario_condition_group_t *group)
{
    return group->type == FULFILLMENT_TYPE_ALL || group->conditions.size > 0;
}

void scenario_condition_type_init(scenario_condition_t *condition)
{
    switch (condition->type) {
        case CONDITION_TYPE_TIME_PASSED:
            scenario_condition_type_time_init(condition);
            break;
        default:
            break;
    }
}

int scenario_condition_type_is_met(scenario_condition_t *condition)
{
    switch (condition->type) {
        case CONDITION_TYPE_BUILDING_COUNT_ACTIVE:
            return scenario_condition_type_building_count_active_met(condition);
        case CONDITION_TYPE_BUILDING_COUNT_ANY:
            return scenario_condition_type_building_count_any_met(condition);
        case CONDITION_TYPE_BUILDING_COUNT_AREA:
            return scenario_condition_type_building_count_area_met(condition);
        case CONDITION_TYPE_CITY_POPULATION:
            return scenario_condition_type_city_population_met(condition);
        case CONDITION_TYPE_COUNT_OWN_TROOPS:
            return scenario_condition_type_count_own_troops_met(condition);
        case CONDITION_TYPE_CUSTOM_VARIABLE_CHECK:
            return scenario_condition_type_custom_variable_check_met(condition);
        case CONDITION_TYPE_DIFFICULTY:
            return scenario_condition_type_difficulty_met(condition);
        case CONDITION_TYPE_MONEY:
            return scenario_condition_type_money_met(condition);
        case CONDITION_TYPE_POPS_UNEMPLOYMENT:
            return scenario_condition_type_population_unemployed_met(condition);
        case CONDITION_TYPE_REQUEST_IS_ONGOING:
            return scenario_condition_type_request_is_ongoing_met(condition);
        case CONDITION_TYPE_RESOURCE_STORAGE_AVAILABLE:
            return scenario_condition_type_resource_storage_available_met(condition);
        case CONDITION_TYPE_RESOURCE_STORED_COUNT:
            return scenario_condition_type_resource_stored_count_met(condition);
        case CONDITION_TYPE_ROME_WAGES:
            return scenario_condition_type_rome_wages_met(condition);
        case CONDITION_TYPE_SAVINGS:
            return scenario_condition_type_savings_met(condition);
        case CONDITION_TYPE_STATS_CITY_HEALTH:
            return scenario_condition_type_stats_city_health_met(condition);
        case CONDITION_TYPE_STATS_CULTURE:
            return scenario_condition_type_stats_culture_met(condition);
        case CONDITION_TYPE_STATS_FAVOR:
            return scenario_condition_type_stats_favor_met(condition);
        case CONDITION_TYPE_STATS_PEACE:
            return scenario_condition_type_stats_peace_met(condition);
        case CONDITION_TYPE_STATS_PROSPERITY:
            return scenario_condition_type_stats_prosperity_met(condition);
        case CONDITION_TYPE_TIME_PASSED:
            return scenario_condition_type_time_met(condition);
        case CONDITION_TYPE_TRADE_ROUTE_OPEN:
            return scenario_condition_type_trade_route_open_met(condition);
        case CONDITION_TYPE_TRADE_ROUTE_PRICE:
            return scenario_condition_type_trade_route_price_met(condition);
        case CONDITION_TYPE_TRADE_SELL_PRICE:
            return scenario_condition_type_trade_sell_price_met(condition);
        case CONDITION_TYPE_TAX_RATE:
            return scenario_condition_type_tax_rate_met(condition);
        default:
            // If we cannot figure condition type (such as with deleted conditions) then default to passed.
            return 1;
    }
}

void scenario_condition_type_delete(scenario_condition_t *condition)
{
    memset(condition, 0, sizeof(scenario_condition_t));
    condition->type = CONDITION_TYPE_UNDEFINED;
}

static void save_conditions_in_group(buffer *buf, const scenario_condition_group_t *group)
{
    const scenario_condition_t *condition;
    array_foreach(group->conditions, condition) {
        buffer_write_i16(buf, condition->type);
        buffer_write_i32(buf, condition->parameter1);
        buffer_write_i32(buf, condition->parameter2);
        buffer_write_i32(buf, condition->parameter3);
        buffer_write_i32(buf, condition->parameter4);
        buffer_write_i32(buf, condition->parameter5);
    }
}

void scenario_condition_group_save_state(buffer *buf, const scenario_condition_group_t *group, int link_type,
    int32_t link_id)
{
    buffer_write_i16(buf, link_type);
    buffer_write_u32(buf, link_id);
    buffer_write_u8(buf, group->type);
    buffer_write_u32(buf, group->conditions.size);
    save_conditions_in_group(buf, group);
}

void scenario_condition_load_state(buffer *buf, scenario_condition_group_t *group, scenario_condition_t *condition)
{
    condition->type = buffer_read_i16(buf);
    condition->parameter1 = buffer_read_i32(buf);
    condition->parameter2 = buffer_read_i32(buf);
    condition->parameter3 = buffer_read_i32(buf);
    condition->parameter4 = buffer_read_i32(buf);
    condition->parameter5 = buffer_read_i32(buf);

    if (condition->type == CONDITION_TYPE_TRADE_SELL_PRICE) {
        condition->parameter1 = resource_remap(condition->parameter1);
    } else if (condition->type == CONDITION_TYPE_RESOURCE_STORED_COUNT) {
        condition->parameter1 = resource_remap(condition->parameter1);
    } else if (condition->type == CONDITION_TYPE_RESOURCE_STORAGE_AVAILABLE) {
        condition->parameter1 = resource_remap(condition->parameter1);
    }
}

void scenario_condition_group_load_state(buffer *buf, scenario_condition_group_t *group,
    int *link_type, int32_t *link_id)
{
    *link_type = buffer_read_i16(buf);
    *link_id = buffer_read_u32(buf);
    group->type = buffer_read_u8(buf);
    unsigned int total_conditions = buffer_read_u32(buf);
    if (!array_init(group->conditions, CONDITION_GROUP_ITEMS_ARRAY_SIZE_STEP, 0, condition_in_use) ||
        !array_expand(group->conditions, total_conditions)) {
        log_error("Unable to create condition group array. The game will now crash.", 0, 0);
    }
    for (unsigned int i = 0; i < total_conditions; i++) {
        scenario_condition_t *condition = array_next(group->conditions);
        scenario_condition_load_state(buf, group, condition);
    }
}

int scenario_condition_uses_custom_variable(const scenario_condition_t *condition, int custom_variable_id)
{
    switch (condition->type) {
        case CONDITION_TYPE_CUSTOM_VARIABLE_CHECK:
            return condition->parameter1 == custom_variable_id;
        default:
            return 0;
    }
}
