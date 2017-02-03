#include "threads/fixed_point.h"
#include "threads/thread.h"
#include <stdio.h>

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
    int fixed_PRI_MAX = convert_to_fixed_point(PRI_MAX, FIXED_POINT_Q);
    int fixed_nice_factor = convert_to_fixed_point(nice * 2, FIXED_POINT_Q);
    int fixed_priority = fixed_PRI_MAX - (recent_cpu / 4) - fixed_nice_factor;
    int int_priority = convert_to_integer_round_nearest(fixed_priority, FIXED_POINT_Q);
    return int_priority;
}

/*
 * Input: Fixed Point and Fixed Point
 * Output: Fixed Point
 */
int calculate_cpu_usage(int recent_cpu, int load_average, int niceness) {
    int fixed_one = convert_to_fixed_point(1, FIXED_POINT_Q);
    int fixed_fraction = divide_x_by_y(2 * load_average,
                                       2 * load_average + fixed_one,
                                       FIXED_POINT_Q);
    int fraction_multiplication = multiply_x_by_y(fixed_fraction,
                                                  recent_cpu,
                                                  FIXED_POINT_Q);
    int fixed_niceness = convert_to_fixed_point(niceness, FIXED_POINT_Q);
    int fixed_new_cpu = fraction_multiplication + fixed_niceness;
    return fixed_new_cpu;
}

/*
 * Input: Fixed Point and Fixed Point
 * Output: Fixed Point
 */
int calculate_load_avg(int load_average, int ready_threads) {
    int fixed_numerator = convert_to_fixed_point(59, FIXED_POINT_Q);
    int fixed_one = convert_to_fixed_point(1, FIXED_POINT_Q);
    int fixed_denominator = convert_to_fixed_point(60, FIXED_POINT_Q);
    int fixed_threads = convert_to_fixed_point(ready_threads, FIXED_POINT_Q);

    int fixed_fraction = 
            divide_x_by_y(fixed_numerator, fixed_denominator, FIXED_POINT_Q);
    int fixed_second_fraction =
            divide_x_by_y(fixed_one, fixed_denominator, FIXED_POINT_Q);

    int fraction_multiplication = 
            multiply_x_by_y(fixed_fraction, load_average, FIXED_POINT_Q);
    int second_fraction_multiplication = multiply_x_by_y(fixed_second_fraction,
                                                         fixed_threads,
                                                         FIXED_POINT_Q);
    return fraction_multiplication + second_fraction_multiplication;
}

