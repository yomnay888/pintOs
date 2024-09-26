#include "FixedPoint.h"    // Include the corresponding header file

// Convert integer to fixed-point representation
fixed_point int_to_fixed(int x) {
    fixed_point result;
    result.value = x*FIXED_SHIFT;
    return result;
}
// Convert fixed-point to integer representation (truncate the fractional part)
int fixed_to_int_truncate(fixed_point x) {
    return x.value /FIXED_SHIFT;
}

// Convert fixed-point to integer representation (round to the nearest integer)
int fixed_to_int_round(fixed_point x) {
  if (x.value >= 0) {
        // For positive x, add f / 2 before dividing
        return (x.value +FIXED_SHIFT/2)/FIXED_SHIFT;
    } else {
        // For negative x, subtract f / 2 before dividing
        return (x.value -FIXED_SHIFT/2)/FIXED_SHIFT;
    }
}

// Addition of two fixed-point numbers
fixed_point fixed_add(fixed_point x, fixed_point y) {
    fixed_point result;
    result.value = x.value + y.value;
    return result;
}
// Addition of one fixed-point number and int 
fixed_point fixed_add_int(fixed_point x, int n) {
    fixed_point result;
    result.value = x.value +n*FIXED_SHIFT;
    return result;
}

// Subtraction of two fixed-point numbers
fixed_point fixed_subtract(fixed_point x, fixed_point y) {
    fixed_point result;
    result.value = x.value - y.value;
    return result;
}

// Subtraction of one fixed-point number and int 
fixed_point fixed_subtract_int(fixed_point x, int n) {
    fixed_point result;
    result.value = x.value -n*FIXED_SHIFT;
    return result;
}
// Multiplication of two fixed-point numbers
fixed_point fixed_multiply(fixed_point x, fixed_point y) {
    fixed_point result;
    result.value = (int32_t)(((int64_t)x.value * y.value)/FIXED_SHIFT); 
    return result;
}
// Multiplication of one fixed-point number and int 
fixed_point fixed_multiply_int(fixed_point x, int n) {
    fixed_point result;
    result.value= (int32_t)(x.value * n);
    
    return result;
}

// Division of two fixed-point numbers
fixed_point fixed_divide(fixed_point x, fixed_point y) {
    fixed_point result;
    result.value = (int32_t)(((int64_t)x.value*FIXED_SHIFT) / y.value); 
    return result;
}
// Division of one fixed-point number and int 
fixed_point fixed_divide_int(fixed_point x, int n) {
    fixed_point result;
    result.value = (int32_t)(x.value / n);
    return result;
}