#include <global_context.h>
#include <ug_parallel.h>
#include <math.h>

/*
Extrair Cálculo de força e reaproveitar nas
implementações paralela e sequencial
*/


Tuple(double, double) get_gforce(Body b1, BodyInfo b2){
    double dx = (b2.m_pos.m_1 - b1.m_pos.m_1);
    double dy = (b2.m_pos.m_2 - b1.m_pos.m_2);
    double r2 = dx*dx + dy*dy;

    if (r2 == 0)
        return (Tuple(double, double)){0.0, 0.0};

    double r = sqrt(r2);
    double inv_r3 = 1.0/(r*r*r);
    double factor = G * b1.m_mass * b2.m_mass * inv_r3;

    return (Tuple(double, double)){factor * dx, factor * dy};
}

void get_acceleration_array(BodyInfo* bodies, size_t num_bodies, Tuple(double, double)* acc_vector){
    Tuple(double, double) force_sum;
    Tuple(double, double) current_gforce;
    for(size_t i = 0; i < num_bodies; i++){
        force_sum = (Tuple(double, double)){0.0, 0.0};

        for(size_t j = 0; j < num_bodies; j++){
            if (i == j) continue;
            
            current_gforce = get_gforce(bodies[i], bodies[j]);
            force_sum.m_1 += current_gforce.m_1;
            force_sum.m_2 += current_gforce.m_2;
        }
        acc_vector[i].m_1 = force_sum.m_1 / bodies[i].m_mass;
        acc_vector[i].m_2 = force_sum.m_2 / bodies[i].m_mass;
    }
}

void update_state(GlobalContext *const state, Tuple(double, double)* acceleration_vector){
    for(int i = 0; i < state->m_num_bodies; i++) {
        state->m_bodies[i].m_pos.m_1 = (
            state->m_bodies[i].m_pos.m_1 +
            state->m_bodies[i].m_spd.m_1 * state->m_dt +
            acceleration_vector[i].m_1 * (state->m_dt * state->m_dt / 2)
        );
        state->m_bodies[i].m_pos.m_2 = (
            state->m_bodies[i].m_pos.m_2 +
            state->m_bodies[i].m_spd.m_2 * state->m_dt +
            acceleration_vector[i].m_2 * (state->m_dt * state->m_dt / 2)
        );
        state->m_bodies[i].m_spd.m_1 = (
            state->m_bodies[i].m_spd.m_1 +
            acceleration_vector[i].m_1 * state->m_dt
        );
        state->m_bodies[i].m_spd.m_2 = (
            state->m_bodies[i].m_spd.m_2 +
            acceleration_vector[i].m_2 * state->m_dt
        );
    }
}

void iterate_states(GlobalContext *const init_state) {
    Tuple(double, double)* acc_vector = malloc(sizeof(Tuple(double, double)) * init_state->m_num_bodies);
    for(int i = 0; i < init_state->m_num_steps; i++) {
        get_acceleration_array(init_state->m_bodies, init_state->m_num_bodies, acc_vector);
        update_state(init_state, acc_vector);
    }
    free(acc_vector);
}