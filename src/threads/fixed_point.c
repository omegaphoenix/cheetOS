#include "threads/fixed_point.h"
#include "threads/thread.h"

int convert_to_fixed_point(int n, int q) {
    int f = 1 << q;
    return n * f;
}

int convert_to_integer_round_zero(int x, int q) {
    int f = 1 << q;
    return x / f;
}

int convert_to_integer_round_nearest(int x, int q) {
    int f = 1 << q;
    if (x >= 0) {
        return (x + f / 2) / f;  
    }
    return (x - f / 2) / f;
}

int multiply_x_by_y(int x, int y, int q) {
    int f = 1 << q;

    return ((int64_t) x) * y / f;
}

int multiply_x_by_n(int x, int n) {
    return x * n;
}

int divide_x_by_y(int x, int y, int q) {
    int f = 1 << q;

    return ((int64_t) x) * f / y;
}

int divide_x_by_n(int x, int n) {
    return x / n;
}

/* 
 * Input: Fixed Point and Integer
 * Output: Integer
 */  
int calculate_priority(int recent_cpu, int nice){
    int fixed_PRI_MAX = convert_to_fixed_point(PRI_MAX, 14);
    int fixed_nice_factor = convert_to_fixed_point(nice * 2, 14);
    int fixed_priority = fixed_PRI_MAX - (recent_cpu / 4) + fixed_nice_factor;

    int int_priority = convert_to_integer_round_nearest(fixed_priority, 14);
    return int_priority;
}


