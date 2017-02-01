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

/*
 * Input: Fixed Point and Fixed Point
 * Output: Fixed Point
 */
int calculate_cpu_usage(int recent_cpu, int load_average, int niceness) {
    int fixed_one = convert_to_fixed_point(1, 14);
    int fixed_fraction = divide_x_by_y(2 * load_average,
                                       2 * load_average + fixed_one,
                                       14);
    int fraction_multiplication = multiply_x_by_y(fixed_fraction,
                                                  recent_cpu,
                                                  14);
    int fixed_niceness = convert_to_fixed_point(niceness, 14);
    int fixed_new_cpu = fraction_multiplication + fixed_niceness;

    return fixed_new_cpu;
}

/*
 * Input: Fixed Point and Fixed Point
 * Output: Fixed Point
 */
int calculate_load_avg(int load_average, int ready_threads) {
    int fixed_numerator = convert_to_fixed_point(59, 14);
    int fixed_one = convert_to_fixed_point(1, 14);
    int fixed_denominator = convert_to_fixed_point(60, 14);
    int fixed_threads = convert_to_fixed_point(ready_threads, 14);

    int fixed_fraction = divide_x_by_y(fixed_numerator, fixed_denominator, 14);
    int fixed_second_fraction = divide_x_by_y(fixed_one, fixed_denominator, 14);

    int fraction_multiplication = multiply_x_by_y(fixed_fraction, load_average, 14);
    int second_fraction_multiplication = multiply_x_by_y(fixed_second_fraction,
                                                         fixed_threads);
    return fraction_multiplication + second_fraction_multiplication;
}

