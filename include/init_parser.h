//init_parser.h
#ifndef INIT_PARSER_H
#define INIT_PARSER_H

#include <stdlib.h>

typedef struct BodyInfo {
  double m_mass;
  double m_pos_x, m_pos_y;
  double m_spd_x, m_spd_y;
} BodyInfo;

typedef struct InitInfo {
  size_t m_num_bodies;
  size_t m_num_steps;
  double m_dt;
  BodyInfo *m_bodies;
} InitInfo;

InitInfo init_parser_parse(char const *const path);
void init_parser_free(InitInfo* ptr_to_auto_var);
void init_parser_log(InitInfo const *const iptr);
char init_parser_is_null(InitInfo const *const iptr);
#endif // INIT_PARSER_H