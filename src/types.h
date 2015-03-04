#include <stdint.h>
#include <stdbool.h>

#define uint8 uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define uint64 uint64_t

#define int8 int8_t
#define int16 int16_t
#define int32 int32_t
#define int64 int64_t

#define f32 float 
#define f64 double 

#define UNUSED(expr) do { (void)(expr); } while (0)

#ifdef MSVC
#define LUMINOS_INLINE __inline
#else
#define LUMINOS_INLINE inline
#endif
