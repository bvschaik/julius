#include "action_handler.h"

#include "scenario/action_types/action_types.h"

void scenario_action_type_init(scenario_action_t *action)
{
    switch (action->type) {
        case ACTION_TYPE_ADJUST_CITY_HEALTH:
            scenario_action_type_city_health_init(action);
            break;
        case ACTION_TYPE_ADJUST_MONEY:
            scenario_action_type_money_add_init(action);
            break;
        case ACTION_TYPE_ADJUST_ROME_WAGES:
            scenario_action_type_rome_wages_init(action);
            break;
        case ACTION_TYPE_ADJUST_SAVINGS:
            scenario_action_type_savings_add_init(action);
            break;
        default:
            break;
    }
}

int scenario_action_type_execute(scenario_action_t *action)
{
    switch (action->type) {
        case ACTION_TYPE_ADJUST_CITY_HEALTH:
            return scenario_action_type_city_health_execute(action);
        case ACTION_TYPE_ADJUST_FAVOR:
            return scenario_action_type_favor_add_execute(action);
        case ACTION_TYPE_ADJUST_ROME_WAGES:
            return scenario_action_type_rome_wages_execute(action);
        case ACTION_TYPE_ADJUST_MONEY:
            return scenario_action_type_money_add_execute(action);
        case ACTION_TYPE_ADJUST_SAVINGS:
            return scenario_action_type_savings_add_execute(action);
        case ACTION_TYPE_CHANGE_ALLOWED_BUILDINGS:
            return scenario_action_type_change_allowed_buildings_execute(action);
        case ACTION_TYPE_CHANGE_RESOURCE_PRODUCED:
            return scenario_action_type_change_resource_produced_execute(action);
        case ACTION_TYPE_EMPIRE_MAP_CONVERT_FUTURE_TRADE_CITY:
            return scenario_action_type_empire_map_convert_future_trade_city_execute(action);
        case ACTION_TYPE_GLADIATOR_REVOLT:
            return scenario_action_type_gladiator_revolt_execute(action);
        case ACTION_TYPE_REQUEST_IMMEDIATELY_START:
            return scenario_action_type_request_immediately_start_execute(action);
        case ACTION_TYPE_SEND_STANDARD_MESSAGE:
            return scenario_action_type_send_standard_message_execute(action);
        case ACTION_TYPE_TRADE_ADJUST_PRICE:
            return scenario_action_type_trade_price_adjust_execute(action);
        case ACTION_TYPE_TRADE_ADJUST_ROUTE_AMOUNT:
            return scenario_action_type_trade_route_amount_execute(action);
        case ACTION_TYPE_TRADE_PROBLEM_LAND:
            return scenario_action_type_trade_problems_land_execute(action);
        case ACTION_TYPE_TRADE_PROBLEM_SEA:
            return scenario_action_type_trade_problems_sea_execute(action);
        case ACTION_TYPE_TRADE_SET_PRICE:
            return scenario_action_type_trade_price_set_execute(action);
        default:
            return 0;
    }
}

void scenario_action_type_delete(scenario_action_t *action)
{
    memset(action, 0, sizeof(scenario_action_t));
    action->type = ACTION_TYPE_UNDEFINED;
}

void scenario_action_type_save_state(buffer *buf, scenario_action_t *action, int link_type, int32_t link_id)
{
    buffer_write_i16(buf, link_type);
    buffer_write_i32(buf, link_id);
    buffer_write_i16(buf, action->type);
    buffer_write_i32(buf, action->parameter1);
    buffer_write_i32(buf, action->parameter2);
    buffer_write_i32(buf, action->parameter3);
    buffer_write_i32(buf, action->parameter4);
    buffer_write_i32(buf, action->parameter5);
}

void scenario_action_type_load_state(buffer *buf, scenario_action_t *action, int *link_type, int32_t *link_id)
{
    *link_type = buffer_read_i16(buf);
    *link_id = buffer_read_i32(buf);
    action->type = buffer_read_i16(buf);
    action->parameter1 = buffer_read_i32(buf);
    action->parameter2 = buffer_read_i32(buf);
    action->parameter3 = buffer_read_i32(buf);
    action->parameter4 = buffer_read_i32(buf);
    action->parameter5 = buffer_read_i32(buf);
}
