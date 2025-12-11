#ifndef _UNIVERSAL_GRAVITY_H
#define _UNIVERSAL_GRAVITY_H

#define G 6.67430e-11

#include <stdio.h>

typedef struct object object;
typedef struct Tuple Tuple;
typedef struct BodyInfo BodyInfo;
typedef struct InitInfo InitInfo;

struct Tuple{
    double m_x;
    double m_y;
};

struct BodyInfo {
  double m_mass;
  double m_pos_x, m_pos_y;
  double m_spd_x, m_spd_y;
};

struct InitInfo {
  size_t m_num_bodies;
  size_t m_num_steps;
  double m_dt;
  BodyInfo *m_bodies;
};


#endif // _UNIVERSAL_GRAVITY_H