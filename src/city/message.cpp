#include "message.h"

static struct {
    struct {
        int pop500;
        int pop1000;
        int pop2000;
        int pop3000;
        int pop5000;
        int pop10000;
        int pop15000;
        int pop20000;
        int pop25000;
    } population_shown;
} data;

void city_message_init_scenario()
{
    // population
    data.population_shown.pop500 = 0;
    data.population_shown.pop1000 = 0;
    data.population_shown.pop2000 = 0;
    data.population_shown.pop3000 = 0;
    data.population_shown.pop5000 = 0;
    data.population_shown.pop10000 = 0;
    data.population_shown.pop15000 = 0;
    data.population_shown.pop20000 = 0;
    data.population_shown.pop25000 = 0;
}

int city_message_mark_population_shown(int population)
{
    int *field = 0;
    switch (population) {
        case 500: field = &data.population_shown.pop500; break;
        case 1000: field = &data.population_shown.pop1000; break;
        case 2000: field = &data.population_shown.pop2000; break;
        case 3000: field = &data.population_shown.pop3000; break;
        case 5000: field = &data.population_shown.pop5000; break;
        case 10000: field = &data.population_shown.pop10000; break;
        case 15000: field = &data.population_shown.pop15000; break;
        case 20000: field = &data.population_shown.pop20000; break;
        case 25000: field = &data.population_shown.pop25000; break;
    }
    if (field && !*field) {
        *field = 1;
        return 1;
    }
    return 0;
}


void city_message_save_state(buffer *population)
{
    // population
    buffer_write_u8(population, 0);
    buffer_write_u8(population, data.population_shown.pop500);
    buffer_write_u8(population, data.population_shown.pop1000);
    buffer_write_u8(population, data.population_shown.pop2000);
    buffer_write_u8(population, data.population_shown.pop3000);
    buffer_write_u8(population, data.population_shown.pop5000);
    buffer_write_u8(population, data.population_shown.pop10000);
    buffer_write_u8(population, data.population_shown.pop15000);
    buffer_write_u8(population, data.population_shown.pop20000);
    buffer_write_u8(population, data.population_shown.pop25000);
}

void city_message_load_state(buffer *population)
{
    // population
    buffer_skip(population, 1);
    data.population_shown.pop500 = buffer_read_u8(population);
    data.population_shown.pop1000 = buffer_read_u8(population);
    data.population_shown.pop2000 = buffer_read_u8(population);
    data.population_shown.pop3000 = buffer_read_u8(population);
    data.population_shown.pop5000 = buffer_read_u8(population);
    data.population_shown.pop10000 = buffer_read_u8(population);
    data.population_shown.pop15000 = buffer_read_u8(population);
    data.population_shown.pop20000 = buffer_read_u8(population);
    data.population_shown.pop25000 = buffer_read_u8(population);
}
