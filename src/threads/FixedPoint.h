#ifndef FIXED_POINT_H
#define FIXED_POINT_H
#include <stdint.h>
#define FIXED_SHIFT  (1<<14)   // for shifting with fraction bits
// Structure to represent a fixed-point number
typedef struct {
    int value;
} fixed_point;
// Function declarations
int fixed_to_int_truncate(fixed_point x);
int fixed_to_int_round(fixed_point x);
fixed_point int_to_fixed(int x);
fixed_point fixed_add(fixed_point x, fixed_point y);
fixed_point fixed_subtract(fixed_point x, fixed_point y);
fixed_point fixed_multiply(fixed_point x, fixed_point y);
fixed_point fixed_divide(fixed_point x, fixed_point y);
fixed_point fixed_subtract_int(fixed_point x, int n);
fixed_point fixed_add_int(fixed_point x, int n);
fixed_point fixed_multiply_int(fixed_point x, int n);
fixed_point fixed_divide_int(fixed_point x, int n);
#endif /* FIXED_POINT_H */
