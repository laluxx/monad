/*
* @file math.c
*/

// FULL TESTS LIST
// #include <stdio.h>
// // acos, asin, atan and atan2 TESTS
// int main() {
//     double val;
//     printf("Input num (-1.0 to 1.0): ");
//     scanf("%lf", &val);
//
//     if (val < -1.0 || val > 1.0) {
//         printf("Error: Input out of bounds for asin.\n");
//         return 1;
//     }
//
//     double result = asin(val);
//     double reconstructed = sin(result);
//     double diff = reconstructed - val;
//
//     printf("asin(%lf) = %.10lf\n", val, result);
//     printf("Check (sin(asin(x)) - x): %.15lf\n", diff);
//
//     result = acos(val);
//     reconstructed = cos(result);
//     diff = reconstructed - val;
//
//     printf("acos(%lf) = %.10lf\n", val, result);
//     printf("Check (cos(acos(x)) - x): %.15lf\n", diff);
//
//     result = atan(val);
//     reconstructed = tan(result);
//     diff = reconstructed - val;
//
//     printf("atan(%lf) = %.10lf\n", val, result);
//     printf("Check (tan(atan(x)) - x): %.15lf\n", diff);
//
//     return 0;
// }
//
// GENERAL TEST
// #include <stdio.h>
// int main(void){
//     double val1;
//     double val2 = 0.0;
//     int e;
//
//     printf("Input the first value: ");
//     int err1 = scanf("%lf", &val1);
//     printf("Input a second value: ");
//     int err2 = scanf("%lf", &val2);
//     printf("\n");
//
//     if ((err1 == 1) && (err2 == 1)) {
//         printf("Result of sqrt:  %lf\n",  sqrt(val1));
//         printf("Result of fabs:  %lf\n",  fabs(val1));
//         printf("Result of sin:   %lf\n",  sin(val1));
//         printf("Result of cos:   %lf\n",  cos(val1));
//         printf("Result of tan:   %lf\n",  tan(val1));
//         printf("Result of asin:  %lf\n",  asin(val1));
//         printf("Result of acos:  %lf\n",  acos(val1));
//         printf("Result of atan:  %lf\n",  atan(val1));
//         printf("Result of atan2: %lf\n",  atan2(val1, val2));
//         printf("Result of exp:   %lf\n",   exp(val1));
//         printf("Result of log:   %lf\n",   log(val1));
//         printf("Result of log10: %lf\n",  log10(val1));
//         printf("Result of pow:   %lf\n",   pow(val1, val2));
//         printf("Result of floor: %lf\n", floor(val1));
//         printf("Result of ceil:  %lf\n",  ceil(val1));
//         printf("Result of sinh:  %lf\n",  sinh(val1));
//         printf("Result of cosh:  %lf\n",  cosh(val1));
//         printf("Result of tanh:  %lf\n",  tanh(val1));
//         printf("Result of fmod:  %lf\n",  fmod(val1, val2));
//         printf("Result of frexp: %lf (exp: %d)\n", frexp(val1, &e), e);
//         printf("Result of ldexp: %lf\n", ldexp(val1, (int)val2));
//
//     } else {
//         printf("Error reading input.\n");
//     }
// }

#include <float.h>
#include <stdbit.h>
#include "math.h"

double sqrt(double x){
    double res;
    asm volatile ("sqrtsd %1, %0" : "=v" (res) : "v" (x));
    return res;
}

double sin(double x){
    double res;
    asm volatile (
        "fsin"
        : "=t" (res)
        : "0" (x)
    );
    return res;
}

double cos(double x){
    double res;
    asm volatile (
        "fcos"
        : "=t" (res)
        : "0" (x)
    );
    return res;
}

double tan(double x){
    double res;
    asm volatile (
        "fptan \n\t"
        "fstp %%st(0)"
        : "=t" (res)
        : "0" (x)
    );
    return res;
}

double fabs(double x) {
    return (x < 0.0) ? -x : x;
}

double asin(double x) {
    double negate = (double)(x < 0);
    x = fabs(x);

    double ret = -0.0187293;
    ret = ret * x + 0.0742610;
    ret = ret * x - 0.2121144;
    ret = ret * x + 1.5707288;
    ret = ret * sqrt(1.0 - x);

    double result = M_PI_2 - ret;

    return result - 2 * negate * result;
}

double acos(double x) {
    return M_PI_2 - asin(x);
}

double atan(double x) {
    return asin(x / sqrt(1.0 + x * x));
}

double atan2(double y, double x) {
    if (x == 0.0 && y == 0.0) return 0.0;

    double r = sqrt(x * x + y * y);

    double angle = acos(x / r);

    double is_negative = (double)(y < 0);
    return angle - 2 * is_negative * angle;
}

double floor(double x) {
    if (isnan(x) || isinf(x)) {
        return x;
    }

    if (fabs(x) >= 4503599627370496.0) {
        return x;
    }

    long long i = (long long)x;

    if (x < (double)i) {
        return (double)(i - 1);
    }

    return (double)i;
}

double ceil(double x) {
    if (isnan(x) || isinf(x)) {
        return x;
    }

    if (fabs(x) >= 4503599627370496.0) {
        return x;
    }

    long long i = (long long)x;

    if (x > (double)i) {
        return (double)(i + 1);
    }

    return (double)i;
}

double exp(double x) {
    return 1.0 + x * (1.0 + x * (0.5 + x * (0.16666666666666666 + x * 0.041666666666666664)));
}

double sinh(double x){
    return (exp(x) - exp(-x)) / 2;
}

double cosh(double x){
    return (exp(x) + exp(-x)) / 2;
}

double tanh(double x){
    return (exp(x) + exp(-x)) / (exp(x) - exp(-x));
}

double fmod(double x, double y) {
    double quotient = x / y;
    long long q_int = (long long)quotient;
    double remainder = x - (y * (double)q_int);

    return remainder;
}

double frexp(double value, int *exp) {
    union { double d; uint64_t u; } u;

    if (value == 0.0) {
        *exp = 0;
        return 0.0;
    }

    u.d = value;

    if ((u.u & DBL_EXP_MASK) == DBL_EXP_MASK) {
        *exp = 0;
        return value;
    }

    if ((u.u & DBL_EXP_MASK) == 0) {
        value *= 1.8014398509481984e16;
        u.d = value;
        *exp = -54;
    } else {
        *exp = 0;
    }

    int raw_exp = (u.u & DBL_EXP_MASK) >> DBL_EXP_SHIFT;
    *exp += (raw_exp - 1022);

    u.u &= DBL_MANT_MASK;
    u.u |= (0x3FE0000000000000ULL);

    return u.d;
}

double ldexp(double x, int exp) {
    union { double d; uint64_t u; } u;
    u.d = x;

    if (exp == 0) return x;
    if (x == 0.0) return x; // Preserves -0.0

    if ((u.u & DBL_EXP_MASK) == DBL_EXP_MASK) return x;

    long long raw_exp = (long long)((u.u & DBL_EXP_MASK) >> DBL_EXP_SHIFT);
    long long new_exp = raw_exp + exp;

    if (new_exp > 0 && new_exp < 2047) {
        u.u &= DBL_MANT_MASK; // Clear old exp
        u.u |= ((uint64_t)new_exp << DBL_EXP_SHIFT);
        return u.d;
    }

    double factor = 1.0;
    if (exp > 0) {
        while (exp > 1023) {
            x *= 8.988465674311579e307;
            exp -= 1023;
        }
        u.u = ((uint64_t)(exp + DBL_EXP_BIAS) << DBL_EXP_SHIFT);
        return x * u.d;
    }
    else {
        while (exp < -1023) {
            x *= 1.1125369292536007e-308;
            exp += 1023;
        }
        u.u = ((uint64_t)(exp + DBL_EXP_BIAS) << DBL_EXP_SHIFT);
        return x * u.d;
    }
}

double log10(double x) {
    if (isnan(x)) return x;
    if (x < 0.0) return NAN;
    if (x == 0.0) return -INFINITY;
    if (isinf(x)) return x;

    union { double d; uint64_t i; } u;
    u.d = x;

    int64_t exp = ((u.i >> 52) & 0x7FF) - 1023;

    u.i = (u.i & 0x000FFFFFFFFFFFFFULL) | 0x3FF0000000000000ULL;
    double m = u.d;

    if (m > SQRT_2) {
        m /= 2.0;
        exp++;
    }

    double z = (m - 1.0) / (m + 1.0);
    double z2 = z * z;

    double poly = z * (2.0 + z2 * (0.66666666666666666 + z2 * (0.4 + z2 * 0.2857142857142857)));

    return (poly * LOG10_E) + ((double)exp * LOG10_2);
}

double log(double x) {
    if (x <= 0.0) {
        if (x == 0.0) return -INFINITY;
        return NAN;
    }

    union { double d; uint64_t i; } u;
    u.d = x;
    if ((u.i & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) {
        return x;
    }

    int64_t exp_bits = (int64_t)(u.i >> 52) & 0x7FF;
    int64_t exp = exp_bits - 1023;

    u.i &= 0x800FFFFFFFFFFFFFULL;
    u.i |= 0x3FF0000000000000ULL;
    double m = u.d;

    double e_d = (double)exp;

    if (m > SQRT_2) {
        m /= SQRT_2;
        e_d += 0.5;
    }

    double t = (m - 1.0) / (m + 1.0);
    double t2 = t * t;

    #define C1  (0.33333333333333333)
    #define C2  (0.20000000000000000)
    #define C3  (0.14285714285714285)
    #define C4  (0.11111111111111111)
    #define C5  (0.09090909090909091)

    double approximation = t2 * (C5 + t2 * (C4 + t2 * (C3 + t2 * (C2 + t2 * C1))));
    double log_m = 2.0 * t * (1.0 + approximation);

    return (e_d * LN_2) + log_m;
}

double pow(double x, double y) {
    if (y == 0.0) return 1.0;
    if (y == 1.0) return x;
    if (isnan(x) || isnan(y)) return NAN;

    if (x == 0.0) {
        if (y < 0.0) return INFINITY;
        return 0.0;
    }

    if (x == 1.0) return 1.0;
    if (y == -1.0) return 1.0 / x;
    if (y == 0.5) {
        if (x < 0.0) return NAN;
        return sqrt(x);
    }

    if (fabs(y) < 9.223372036854776e18 && y == (double)(int64_t)y) {
        int64_t n = (int64_t)y;

        if (n == 2) return x * x;
        if (n == 3) return x * x * x;
        if (n == 4) { double t = x*x; return t*t; }
        if (n == -2) return 1.0 / (x * x);

        if (n < 0) {
            x = 1.0 / x;
            n = -n;
        }

        double result = 1.0;
        while (n > 0) {
            if (n & 1) {
                result *= x;
            }
            x *= x;
            n >>= 1;
        }
        return result;
    }

    if (x < 0.0) return NAN;
    return exp(y * log(x));
}


