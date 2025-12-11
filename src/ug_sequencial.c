#include <ug_sequencial.h>
#include <math.h>

Tuple get_gforce(BodyInfo b1, BodyInfo b2){
    double dx = (b2.m_pos_x-b1.m_pos_x);
    double dy = (b2.m_pos_y-b1.m_pos_y);
    double r2 = dx*dx + dy*dy;

    if (r2 == 0)
        return (Tuple){0.0, 0.0};

    double r = sqrt(r2);
    double inv_r3 = 1.0/(r*r*r);
    double factor = G * b1.m_mass * b2.m_mass * inv_r3;

    return (Tuple){factor * dx, factor * dy};
}

Tuple* get_acceleration_array(BodyInfo* bodies, size_t num_bodies){
    // Se não precisar da força, dá pra remover a massa do fator de força e da acelação
    Tuple* acc = malloc(sizeof(Tuple) * num_bodies);
    for(int i = 0; i < num_bodies; i++){
        Tuple force_sum = {0.0, 0.0};

        for(int j = 0; j < num_bodies; j++){
            if (i == j) continue;
            
            Tuple current_gforce = get_gforce(bodies[i], bodies[j]);
            force_sum.m_x += current_gforce.m_x;
            force_sum.m_y += current_gforce.m_y;
        }
        acc[i].m_x = force_sum.m_x / bodies[i].m_mass;
        acc[i].m_y = force_sum.m_y / bodies[i].m_mass;
    }
    return acc;
}

void update_state(InitInfo state, Tuple* acceleration_vector){
    for(int i = 0; i < state.m_num_bodies; i++){
        state.m_bodies[i].m_pos_x = (
            state.m_bodies[i].m_pos_x +
            state.m_bodies[i].m_spd_x * state.m_dt +
            acceleration_vector[i].m_x * (state.m_dt * state.m_dt / 2)
        );
        state.m_bodies[i].m_pos_y = (
            state.m_bodies[i].m_pos_y +
            state.m_bodies[i].m_spd_y * state.m_dt +
            acceleration_vector[i].m_y * (state.m_dt * state.m_dt / 2)
        );
    }
}

InitInfo iterate_states(InitInfo init_state){
    for(int i = 0; i < init_state.m_num_steps; i++){
        Tuple* acc_vector = get_acceleration_array(init_state.m_bodies, init_state.m_num_bodies);
        update_state(init_state, acc_vector);
    }
    return init_state;
}