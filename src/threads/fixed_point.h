/*! \file fixed_point.h
 *
 * Function definitions for using fixed point real arithmetic.
 * We are using the 17.14 notation for this arithmetic.
 */

#ifndef FIXED_POINT_H_
#define FIXED_POINT_H_

#define FIXED_POINT_Q 14
#define FIXED_ONE 1 << 14

/* Converts a number into p.q format */
int convert_to_fixed_point(int n, int q);

/* Converts a p.q number to an integer, rounded to zero if necesary */
int convert_to_integer_round_zero(int x, int q);

/* Converts a p.q number to an integer, rounded to nearest if necessary */
int convert_to_integer_round_nearest(int x, int q);

/* Addition and subtraction are kept in fixed point numbers */

/* Multiplies two fixed point numbers. */
int multiply_x_by_y(int x, int y, int q);

/* Multiplies fixed point number with integer */
int multiply_x_by_n(int x, int n);

/* Divides two fixed point numbers */
int divide_x_by_y(int x, int y, int q);

/* Divides a fixed point number by an integer */
int divide_x_by_n(int x, int n);


/* 
 * The following functions will involve the calculation of
 * priority, cpu_usage, and load balancing
 */

/* Calculates the new priority of the thread given cpu usage
   and a niceness.

   Input:
   recent_cpu: Fixed Point
   nice:       Integer

   Return:
   priority:   Integer
*/
int calculate_priority(int recent_cpu, int nice);

/* Calculates the new cpu_usage of the thread given cpu usage
   and a load_average.

   Input:
   recent_cpu:    Fixed Point
   load_average:  Fixed Point

   Return:
   recent_cpu:    Fixed Point
*/
int calculate_cpu_usage(int recent_cpu, int load_average, int niceness);

/* Calculates the new load_average of the thread given previous
   load_average and number of ready threads.

   Input:
   load_avg:       Fixed Point
   ready_threads:  Integer

   Return:
   load_average:    Fixed Point
*/
int calculate_load_avg(int load_average, int ready_threads);

#endif /* FIXED_POINT_H_ */