#ifndef TUPLE_TEMPLATE_H
#define TUPLE_TEMPLATE_H

#include <macros.h>

#define Tuple(...) CONCAT_ALL(tuple__, __VA_ARGS__, __)
#define TUPLE_FIELD(TYPE, NUMBER) TYPE m_##NUMBER;

#define DECL_TUPLE(...) \
typedef struct Tuple(__VA_ARGS__) { \
  MAP(TUPLE_FIELD, __VA_ARGS__) \
} Tuple(__VA_ARGS__)

#endif // TUPLE_TEMPLATE_H