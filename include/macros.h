#ifndef MACROS_H
#define MACROS_H

#include <sys/types.h>

#define deref_as(TYPE, DATA) (*(TYPE*)(DATA))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define CONCAT_SIMPLE(A, B) A##B
#define CONCAT(A, B) CONCAT_SIMPLE(A, B)

#define STRINGIZE_SIMPLE(A) #A
#define STRINGIZE(A) STRINGIZE_SIMPLE(A)

#define MACRO_VAR(BASE_NAME) CONCAT(BASE_NAME, __LINE__)

#ifdef LOG_CODE_ENABLED
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#define SHORT_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(fmt, ...) printf("[LOG F%s L%d] -> " fmt "\n", SHORT_FILE, __LINE__, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) 
#endif

#define COUNT_IMPL(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, N, ...) N
#define COUNT(...) COUNT_IMPL(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define APPLY1(macro, x) macro(x)
#define APPLY2(macro, x, ...) macro(x) APPLY1(macro, __VA_ARGS__)
#define APPLY3(macro, x, ...) macro(x) APPLY2(macro, __VA_ARGS__)
#define APPLY4(macro, x, ...) macro(x) APPLY3(macro, __VA_ARGS__)
#define APPLY5(macro, x, ...) macro(x) APPLY4(macro, __VA_ARGS__)
#define APPLY6(macro, x, ...) macro(x) APPLY5(macro, __VA_ARGS__)
#define APPLY7(macro, x, ...) macro(x) APPLY6(macro, __VA_ARGS__)
#define APPLY8(macro, x, ...) macro(x) APPLY7(macro, __VA_ARGS__)
#define APPLY9(macro, x, ...) macro(x) APPLY8(macro, __VA_ARGS__)
#define APPLY10(macro, x, ...) macro(x) APPLY9(macro, __VA_ARGS__)
#define APPLY11(macro, x, ...) macro(x) APPLY10(macro, __VA_ARGS__)
#define APPLY12(macro, x, ...) macro(x) APPLY11(macro, __VA_ARGS__)

#define FOR_EACH(macro, ...) \
  CONCAT(APPLY, COUNT(__VA_ARGS__))(macro, __VA_ARGS__)

#define CONCAT_1(a) a
#define CONCAT_2(a,b) CONCAT(a,b)
#define CONCAT_3(a,b,c) CONCAT(CONCAT_2(a,b),c)
#define CONCAT_4(a,b,c,d) CONCAT(CONCAT_3(a,b,c),d)
#define CONCAT_5(a,b,c,d,e) CONCAT(CONCAT_4(a,b,c,d),e)
#define CONCAT_6(a,b,c,d,e,f) CONCAT(CONCAT_5(a,b,c,d,e),f)
#define CONCAT_7(a,b,c,d,e,f,g) CONCAT(CONCAT_6(a,b,c,d,e,f),g)
#define CONCAT_8(a,b,c,d,e,f,g,h) CONCAT(CONCAT_7(a,b,c,d,e,f,g),h)
#define CONCAT_9(a,b,c,d,e,f,g,h,i) CONCAT(CONCAT_8(a,b,c,d,e,f,g,h),i)
#define CONCAT_10(a,b,c,d,e,f,g,h,i,j) CONCAT(CONCAT_9(a,b,c,d,e,f,g,h,i),j)
#define CONCAT_11(a,b,c,d,e,f,g,h,i,j,k) CONCAT(CONCAT_10(a,b,c,d,e,f,g,h,i,j),k)
#define CONCAT_12(a,b,c,d,e,f,g,h,i,j,k,l) CONCAT(CONCAT_11(a,b,c,d,e,f,g,h,i,j,k),l)
#define CONCAT_ALL(...) CONCAT(CONCAT_, COUNT(__VA_ARGS__))(__VA_ARGS__)

#define MAP_1(macro, a1) \
  macro(a1, 1)

#define MAP_2(macro, a1,a2) \
  macro(a1,1) macro(a2,2)

#define MAP_3(macro, a1,a2,a3) \
  macro(a1,1) macro(a2,2) macro(a3,3)

#define MAP_4(macro, a1,a2,a3,a4) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4)

#define MAP_5(macro, a1,a2,a3,a4,a5) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5)

#define MAP_6(macro, a1,a2,a3,a4,a5,a6) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6)

#define MAP_7(macro, a1,a2,a3,a4,a5,a6,a7) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7)

#define MAP_8(macro, a1,a2,a3,a4,a5,a6,a7,a8) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7) macro(a8,8)

#define MAP_9(macro, a1,a2,a3,a4,a5,a6,a7,a8,a9) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7) macro(a8,8) macro(a9,9)

#define MAP_10(macro, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7) macro(a8,8) macro(a9,9) macro(a10,10)

#define MAP_11(macro, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7) macro(a8,8) macro(a9,9) macro(a10,10) macro(a11,11)

#define MAP_12(macro, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
  macro(a1,1) macro(a2,2) macro(a3,3) macro(a4,4) macro(a5,5) macro(a6,6) macro(a7,7) macro(a8,8) macro(a9,9) macro(a10,10) macro(a11,11) macro(a12,12)

#define MAP(MACRO, ...) CONCAT(MAP_, COUNT(__VA_ARGS__))(MACRO, __VA_ARGS__)

#endif // MACROS_H