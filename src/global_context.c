#include <global_context.h>

#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>

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
static inline uint64_t cast_u64(const char *str) {
  char *end;
  errno = 0;
  unsigned long long tmp = strtoull(str, &end, 10);
  if (errno == ERANGE
    || end == str
    || *end != '\0')
    return UINT64_MAX;
  if (tmp > UINT64_MAX)
    return UINT64_MAX;
  return (uint64_t)tmp;
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

inline BodyInfo* global_ctx_access_body(GlobalContext const *const iptr, uint64_t idx) {
  return idx >= iptr->m_num_bodies ? NULL : iptr->m_bodies + (5 * idx);
}

inline double* global_ctx_body_access_member(BodyInfo *const bip, BodyInfoMemberIdx idx) {
  return (idx < 0 || idx > 4) ? NULL : bip + idx;
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
  GlobalContext retval      = PARSE_ERROR;  //INTERNAL PTR NEEDS FREE (in end if error)
  FILE *fptr                = NULL;         //NEEDS FREE (in end)
  uint64_t temp_u64         = 0;
  size_t temp_size          = 0;
  char *temp_char_ptr       = NULL;
  char *line_read           = NULL;         //NEEDS FREE (in end)
  char *header_param_str    = NULL;         //NEEDS FREE (before assignment and in end)
  char *header_arg_val_str  = NULL;         //NEEDS FREE (before assignment and in end)
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
      if((temp_u64 = cast_u64(header_arg_val_str)) == UINT64_MAX)
        RET_ERROR;
      retval.m_num_steps = temp_u64;
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
      if((temp_u64 = cast_u64(header_arg_val_str)) == UINT64_MAX)
        RET_ERROR;
      retval.m_num_bodies = temp_u64;
      nbodies_set = 1;
    }
    else
      RET_ERROR;
  }

  //BODIES PARSING
  retval.m_bodies = malloc(sizeof(double) * retval.m_num_bodies * 5);
  if(! retval.m_bodies)
    RET_ERROR;
  for(uint64_t i = 0; i < retval.m_num_bodies; i++) {
    if(getline(&line_read, &temp_size, fptr) == -1)
      RET_ERROR;
    BodyInfo *const info = global_ctx_access_body(&retval, i);
    
    temp_double = cast_double(line_read, &temp_char_ptr);
    if(temp_double == DBL_MAX || *temp_char_ptr != ' ')
      RET_ERROR;
    *global_ctx_body_access_member(info, BI_MASS) = temp_double;

    temp_double = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    if(temp_double == DBL_MAX || *temp_char_ptr != ' ')
      RET_ERROR;
    *global_ctx_body_access_member(info, BI_POS_X) = temp_double;
    
    temp_double = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    if(temp_double == DBL_MAX || *temp_char_ptr != ' ')
      RET_ERROR;
    *global_ctx_body_access_member(info, BI_POS_Y) = temp_double;

    temp_double = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    if(temp_double == DBL_MAX || *temp_char_ptr != ' ')
      RET_ERROR;
    *global_ctx_body_access_member(info, BI_SPD_X) = temp_double;

    temp_double = cast_double(temp_char_ptr + 1, &temp_char_ptr);
    if(temp_double == DBL_MAX || (*temp_char_ptr != '\n' && *temp_char_ptr != '\0'))
      RET_ERROR;
    *global_ctx_body_access_member(info, BI_SPD_Y) = temp_double;
  }
  RET_SUCCESS;
}

#undef PARSE_ERROR
#undef RET

inline void global_ctx_free(GlobalContext *const ptr_to_auto_var) {
  free(ptr_to_auto_var->m_bodies);
  ptr_to_auto_var->m_bodies = NULL;
  ptr_to_auto_var->m_num_bodies = 0;
  ptr_to_auto_var->m_dt = 0.0;
  ptr_to_auto_var->m_num_steps = 0;
}

void global_ctx_log(const GlobalContext *const iptr) {
  if(! iptr || ! iptr->m_bodies)
    return;
  printf("GlobalContext {num_bodies = %" PRIu64 ", num_steps = %" PRIu64 ", dt = %.3lf, bodies ="
    , iptr->m_num_bodies
    , iptr->m_num_steps
    , iptr->m_dt);
  for(uint64_t i = 0; i < iptr->m_num_bodies; i++) {
    BodyInfo *const info = global_ctx_access_body(iptr, i);
    printf("\n\tBodyInfo {mass = %.3e, pos = [%.3e, %.3e], spd = [%.3e, %.3e]},"
      , *global_ctx_body_access_member(info, BI_MASS)
      , *global_ctx_body_access_member(info, BI_POS_X)
      , *global_ctx_body_access_member(info, BI_POS_Y)
      , *global_ctx_body_access_member(info, BI_SPD_X)
      , *global_ctx_body_access_member(info, BI_SPD_Y));
  }   
  printf("}");
}

inline char global_ctx_is_null(GlobalContext const *const iptr) {
  return (iptr == NULL || iptr->m_bodies == NULL);
}