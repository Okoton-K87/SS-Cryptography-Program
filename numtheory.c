//numtheory.c

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>

#include "numtheory.h"
#include "randstate.h"

//for testing
#include <stdlib.h>

//-----------------------------------------gcd--------------------------------------
//mpz version gcd
void gcd(mpz_t g, mpz_t a, mpz_t b) {
    // declare
    mpz_t temp, copy_a, copy_b;
    // initialize
    mpz_inits(temp, copy_a, copy_b, NULL);
    // copy over
    mpz_set(copy_a, a);
    mpz_set(copy_b, b);

    //compare b and 0
    while (mpz_cmp_ui(copy_b, 0) != 0) {
        // t = b;
        mpz_set(temp, copy_b);
        // b = a % b;
        mpz_mod(copy_b, copy_a, copy_b);
        // a = t;
        mpz_set(copy_a, temp);
    }
    // dont return a; sort it in d instead
    mpz_set(g, copy_a);
    mpz_clears(temp, copy_a, copy_b, NULL);
}

//regular version gcd
// int r_gcd(int a, int b) {
//     int t;
//     while (b != 0) {
//         t = b;
//         b = a % b;
//         a = t;
//     }
//     return a;
// }

//----------------------------------------mod_inverse-------------------------------
//mpz version mod_inverse
void mod_inverse(mpz_t o, mpz_t a, mpz_t n) {
    // int r = n, r0 = a, t = 0, t0 = 1, q;
    mpz_t r, r1, t, t1, q, q_value, current_r, current_r1, q_times_r1, current_t, q_times_t1;
    mpz_inits(
        r, r1, t, t1, q, q_value, current_r, current_r1, q_times_r1, current_t, q_times_t1, NULL);

    mpz_set(r, n);
    mpz_set(r1, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t1, 1);

    while (mpz_cmp_ui(r1, 0) != 0) {
        // q = r / r0;
        mpz_fdiv_q(q_value, r, r1);
        mpz_set(q, q_value);

        // holding r, r1 original value
        mpz_set(current_r, r);
        mpz_set(current_r1, r1);

        // r = r0;
        mpz_set(r, current_r1);
        // r0 = r - q * r0;
        mpz_mul(q_times_r1, q, current_r1);
        mpz_sub(r1, current_r, q_times_r1);

        // holding t, t1 original value
        mpz_set(current_t, t);
        // mpz_set(current_t1, t1);
        // t = t0;
        mpz_set(t, t1);
        // t0 = t - q * t0;
        mpz_mul(q_times_t1, q, t1);
        mpz_sub(t1, current_t, q_times_t1);
    }
    // if (r > 1)
    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(o, 0);
        // mpz_clears(r, r1, t, t1, q, q_value, current_r, current_r1, q_times_r1, current_t, q_times_t1, NULL);
    } else if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(t, t, n);
        mpz_set(o, t);
        // mpz_clears(r, r1, t, t1, q, q_value, current_r, current_r1, q_times_r1, current_t, q_times_t1, NULL);
    } else {
        mpz_set(o, t);
    }

    mpz_clears(
        r, r1, t, t1, q, q_value, current_r, current_r1, q_times_r1, current_t, q_times_t1, NULL);
}

//regular version mod_inverse
// int r_mod_inverse(int a, int n) {
//     int r = n, r0 = a, t = 0, t0 = 1, q;
//     while (r0 != 0) {
//         q = r / r0;
//         r = r0;
//         r0 = r - q * r0;
//         t = t0;
//         t0 = t - q * t0;
//     }
//     if (r > 1) {
//         return -1; // no inverse
//     }
//     if (t < 0) {
//         t += n;
//     }
//     return t;
// }

//----------------------------------------pow_mod-----------------------------------
//mpz version pow_mod
void pow_mod(mpz_t o, mpz_t a, mpz_t d, mpz_t n) {
    mpz_t v, p, copy_d, v_times_p, p_times_p, d_over_2;
    mpz_inits(v, p, copy_d, v_times_p, p_times_p, d_over_2, NULL);

    // int v = 1;
    mpz_set_ui(v, 1);
    // int p = a;
    mpz_set(p, a);
    //a copy of d
    mpz_set(copy_d, d);

    //https://gmplib.org/manual/Integer-Comparisons
    while (mpz_sgn(copy_d) > 0) {
        //https://gmplib.org/manual/Miscellaneous-Integer-Functions
        if (mpz_odd_p(copy_d) != 0) {
            mpz_mul(v_times_p, v, p);
            mpz_mod(v, v_times_p, n);
        }
        mpz_mul(p_times_p, p, p);
        mpz_mod(p, p_times_p, n);
        mpz_fdiv_q_ui(d_over_2, copy_d, 2);
        mpz_set(copy_d, d_over_2);
    }
    mpz_set(o, v);
    mpz_clears(v, p, copy_d, v_times_p, p_times_p, d_over_2, NULL);
}

//regular version pow_mod
// int r_pow_mod(int a, int d, int n) {
//     int v = 1;
//     int p = a;
//     while (d > 0) {
//         if (d % 2 == 1) {
//             v = (v * p) % n;
//         }
//         p = (p * p) % n;
//         d = d / 2;
//     }
//     return v;
// }

//----------------------------------------is_prime----------------------------------
//mpz version is_prime
bool is_prime(mpz_t n, uint64_t iters) {
    //check extrem condition where If n is even or less than 2, it is not prime
    if (mpz_cmp_ui(n, 2) == 0) {
        return true;
    }

    if (mpz_cmp_ui(n, 3) == 0) {
        return true;
    }

    if (mpz_even_p(n) != 0 || mpz_cmp_ui(n, 2) < 0) {
        return false;
    }

    mpz_t rand_num, n_minus_1, copy_n_minus_1, n_minus_3, y, ui_2;
    mpz_inits(rand_num, n_minus_1, copy_n_minus_1, n_minus_3, y, ui_2, NULL);

    //r = n - 1;
    mpz_sub_ui(n_minus_1, n, 1);

    //copy n-1 for testing, which is r
    mpz_sub_ui(copy_n_minus_1, n, 1);

    mpz_sub_ui(n_minus_3, n, 3);
    mpz_set_ui(ui_2, 2);

    //int s = 0,
    uint64_t s = 0;
    uint64_t j;

    while (mpz_even_p(copy_n_minus_1) != 0) {
        mpz_fdiv_q(copy_n_minus_1, copy_n_minus_1, ui_2);
        s += 1;
    }

    for (uint64_t i = 0; i < iters; i++) {
        //rand num from 0 to n - 4
        mpz_urandomm(rand_num, state, n_minus_3);
        //add to so that rand num is from 2 to n -2
        mpz_add_ui(rand_num, rand_num, 2);

        pow_mod(y, rand_num, copy_n_minus_1, n);

        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, n_minus_1) != 0) {
            j = 1;
            while (j <= (s - 1) && mpz_cmp(y, n_minus_1) != 0) {
                pow_mod(y, y, ui_2, n);
                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(rand_num, n_minus_1, copy_n_minus_1, n_minus_3, y, ui_2, NULL);
                    return false;
                }
                j += 1;
            }
            if (mpz_cmp(y, n_minus_1) != 0) {
                mpz_clears(rand_num, n_minus_1, copy_n_minus_1, n_minus_3, y, ui_2, NULL);
                return false;
            }
        }
    }
    mpz_clears(rand_num, n_minus_1, copy_n_minus_1, n_minus_3, y, ui_2, NULL);
    return true;
}

// regular version is_prime
// int r_is_prime(int n, int k) {
//     // If n is even or less than 2, it is not prime
//     if (n < 2 || n % 2 == 0) {
//         return 0;
//     }

//     int s = 0, r = n - 1;
//     while (r % 2 == 0) {
//         r /= 2;
//         s++;
//     }

//     for (int i = 0; i < k; i++) {
//         int a = random() % (n - 3) + 2; // Choose a random number between 2 and n-2
//         int y = r_pow_mod(a, r, n);
//         if (y == 1 || y == n - 1) {
//             continue;
//         }
//         for (int j = 1; j < s; j++) {
//             y = r_pow_mod(y, 2, n);
//             if (y == 1) {
//                 return 0;
//             }
//             if (y == n - 1) {
//                 break;
//             }
//         }
//         if (y != n - 1) {
//             return 0;
//         }
//     }
//     return 1;
// }

//------------------------------------make_prime------------------------------------
//mpz version make_prime
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    // mpz_urandomb(p, state, bits);
    // && mpz_sizeinbase(p, 2) < bits
    bool checker = false;
    while (checker == false) {
        //generate a random number from 0 to 2^bits - 1
        mpz_urandomb(p, state, bits + 1);
        //check the number of bits of the random number
        //if is less than the number of bits
        if (mpz_sizeinbase(p, 2) < bits + 1) {
            //go back and retart
            continue;
        }
        checker = is_prime(p, iters);
    }
}
