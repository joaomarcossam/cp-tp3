#include <global_context.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>

/* FILE FORMAT

N: <size_t>
N_STEPS: <size_t>
dt: <double>
<mass : double> <pos_x : double> <pos_y : double> <vel_x : double> <vel_y : double> {N}

typedef struct GlobalContext {
  size_t m_num_bodies;
  size_t m_num_steps;
  double m_dt;
  BodyInfo *m_bodies;
} GlobalContext;

*/

//Works only on null terminated strings
//Char_read arg may be null if you don't want to know how many chars were read
static char* str_aux_copy_until(char const *const str, char const character, size_t *char_read) {
  size_t buffer_insert  = 0;
  size_t buffer_cap     = 16;
  char *buffer          = malloc(buffer_cap);
  char *temp_ptr        = NULL;
  if(! buffer)
    return NULL;
  for(size_t i = 0; str[i] != '\0'; i += 1) {
    if(str[i] == character)
      break;
    if(buffer_insert == buffer_cap) {
      buffer_cap *= 2;
      temp_ptr = realloc(buffer, buffer_cap);
      if(! temp_ptr) {
        free(buffer);
        return NULL;
      }
      buffer = temp_ptr;
    }
    buffer[buffer_insert++] = str[i];
  }
  if(buffer_insert == buffer_cap) {
    temp_ptr = realloc(buffer, buffer_cap + 1);
    if(! temp_ptr) {
      free(buffer);
      return NULL;
    }
    buffer = temp_ptr;
  }
  buffer[buffer_insert] = '\0';
  if(char_read)
    *char_read = buffer_insert;
  return buffer;
}

//Expects a null terminated string
static inline size_t cast_size(char const *const str) {
  char *temp;
  unsigned long ret = strtoul(str, &temp, 10);
  if(ret == ULONG_MAX
    || temp == str
    || *temp != '\0')
    return SIZE_MAX;
  return ret;
}

static inline double cast_double(char const *const str, char **end_ptr) {
  char *temp;
  errno = 0;
  double val = strtod(str, &temp);
  if(temp == str
    || errno != 0)
    return DBL_MAX;
  if(end_ptr)
    *end_ptr = temp;
  return val;
}

#define PARSE_ERROR (GlobalContext){0, 0, 0.0, NULL}
#define RET_ERROR \
  do { \
    free(retval.m_bodies); \
    if(fptr != NULL) \
      fclose(fptr); \
    free(line_read); \
    free(header_param_str); \
    free(header_arg_val_str); \
    return PARSE_ERROR; \
  } while(0)

#define RET_SUCCESS \
  do { \
    if(fptr != NULL) \
      fclose(fptr); \
    free(line_read); \
    free(header_param_str); \
    free(header_arg_val_str); \
    return retval; \
  } while(0)

GlobalContext global_ctx_parse(const char *const path) {
  //VAR DECLARATIONS
  GlobalContext retval      = PARSE_ERROR; //INTERNAL PTR NEEDS FREE (in end)
  FILE *fptr                = NULL; //NEEDS FREE (in end)
  size_t temp_size          = 0;
  char *temp_char_ptr       = NULL;
  char *line_read           = NULL; //NEEDS FREE (in end)
  char *header_param_str    = NULL; //NEEDS FREE (before assignment and in end)
  char *header_arg_val_str  = NULL; //NEEDS FREE (before assignment and in end)
  char nbodies_set          = 0;
  char nsteps_set           = 0;
  char dt_set               = 0;
  double temp_double        = 0.0;

  //HEADER PARSING
  fptr = fopen(path, "r");
  if(! fptr)
    RET_ERROR;
  for(char op_line = 0; op_line < 3; op_line++) {
    if(getline(&line_read, &temp_size, fptr) == -1)
      RET_ERROR;
    free(header_param_str);
    free(header_arg_val_str);
    header_param_str = str_aux_copy_until(line_read, ':', &temp_size);
    header_arg_val_str = str_aux_copy_until(line_read + temp_size + 1, '\n', &temp_size);
    if((! header_param_str) || (! header_arg_val_str))
      RET_ERROR;
    if(! nsteps_set && strncmp("N_STEPS", header_param_str, 7) == 0) {
      if((temp_size = cast_size(header_arg_val_str)) == SIZE_MAX)
        RET_ERROR;
      retval.m_num_steps = temp_size;
      nsteps_set = 1;
    }
    else if(! dt_set && strncmp("dt", header_param_str, 2) == 0) {
      temp_double = strtod(header_arg_val_str, &temp_char_ptr);
      if(*temp_char_ptr != '\0')
        RET_ERROR;
      retval.m_dt = temp_double;
      dt_set = 1;
    }
    else if(! nbodies_set && header_param_str[0] == 'N') {
      if((temp_size = cast_size(header_arg_val_str)) == SIZE_MAX)
        RET_ERROR;
      retval.m_num_bodies = temp_size;
      nbodies_set = 1;
    }
    else
      RET_ERROR;
  }

  //BODIES PARSING
  retval.m_bodies = malloc(sizeof(BodyInfo) * retval.m_num_bodies);
  if(! retval.m_bodies)
    RET_ERROR;
  for(size_t i = 0; i < retval.m_num_bodies; i++) {
    if(getline(&line_read, &temp_size, fptr) == -1)
      RET_ERROR;
    retval.m_bodies[i].m_mass   = cast_double(line_read, &temp_char_ptr);
    retval.m_bodies[i].m_pos.m_1  = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    retval.m_bodies[i].m_pos.m_2  = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    retval.m_bodies[i].m_spd.m_1  = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    retval.m_bodies[i].m_spd.m_2  = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    if(retval.m_bodies[i].m_mass   == DBL_MAX
      || retval.m_bodies[i].m_pos.m_1  == DBL_MAX
      || retval.m_bodies[i].m_pos.m_2  == DBL_MAX
      || retval.m_bodies[i].m_spd.m_1  == DBL_MAX
      || retval.m_bodies[i].m_spd.m_2  == DBL_MAX)
      RET_ERROR;
  }
  RET_SUCCESS;
}

#undef PARSE_ERROR
#undef RET

inline void global_ctx_free(GlobalContext *ptr_to_auto_var) {
  free(ptr_to_auto_var->m_bodies);
  ptr_to_auto_var->m_bodies = NULL;
  ptr_to_auto_var->m_num_bodies = 0;
  ptr_to_auto_var->m_dt = 0.0;
  ptr_to_auto_var->m_num_steps = 0;
}

void global_ctx_log(const GlobalContext *const iptr) {
  if(! iptr || ! iptr->m_bodies)
    return;
  printf("GlobalContext {num_bodies = %zu, num_steps = %zu, dt = %.3lf, bodies ="
    , iptr->m_num_bodies
    , iptr->m_num_steps
    , iptr->m_dt);
  for(size_t i = 0; i < iptr->m_num_bodies; i++)
    printf("\n\tBodyInfo {mass = %.3e, pos = [%.3e, %.3e], spd = [%.3e, %.3e]},"
      , iptr->m_bodies[i].m_mass
      , iptr->m_bodies[i].m_pos.m_1
      , iptr->m_bodies[i].m_pos.m_2
      , iptr->m_bodies[i].m_spd.m_1
      , iptr->m_bodies[i].m_spd.m_2);
  printf("}");
}

inline char global_ctx_is_null(GlobalContext const *const iptr) {
  return (iptr == NULL || iptr->m_bodies == NULL);
}