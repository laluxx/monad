/*
 * @file math.h
 */

#ifndef MATH_H
#define MATH_H


/* CLASSIFICATION MACRO */

/*
 * @brief Define constants for fpclassify return values (These values are arbitrary in the standard, usually integers)
 */
enum {
    FP_NAN,
    FP_INFINITE,
    FP_ZERO,
    FP_NORMAL,
    FP_SUBNORMAL,
};

/*
 * @brief isnan(x)
 */
#define isnan(x) ((x) != (x))

/*
 * @brief isinf(x)
 */
#define isinf(x) ((x) != 0.0 && (x) * 2.0 == (x))

/*
 * @brief isfinite(x)
 */
#define isfinite(x) (!(naive_isinf(x)) && !(naive_isnan(x)))

/*
 * @brief signbit(x)
 */
#define signbit(x) ((x) < 0.0 || ((x) == 0.0 && (1.0 / (x)) < 0.0))

/*
 * @brief isnormal(x)
 */
#define isnormal(x) (isfinite(x) && (x) != 0.0)

/*
 * @brief fpclassify(x)
 */
#define fpclassify(x) \
    (isnan(x)       ? FP_NAN            : \
    isinf(x)        ? FP_INFINITE       : \
    (x) == 0.0            ? FP_ZERO     : \
    (fabs(x) < FLT_MIN) ? FP_SUBNORMAL  : \
                             FP_NORMAL)

/* COMPARISON MACROS */

/**
 * @brief The anchor macro. Returns non-zero (true) if x or y is NaN.
 * @note isunordered(x, y)
 */
#define isunordered(x, y) (isnan(x) || isnan(y))

/**
 * @brief Returns non-zero if x > y.
 * @note isgreater(x, y)
 * @details Guarded by !isunordered to ensure NaNs return 0 and do not raise exceptions.
 */
#define isgreater(x, y)       (!isunordered((x), (y)) && ((x) > (y)))

/**
 * @brief Returns non-zero if x < y.
 * @note isless(x, y)
 */
#define isless(x, y)          (!isunordered((x), (y)) && ((x) < (y)))

/**
 * @brief Returns non-zero if x >= y.
 * @note isgreaterequal(x, y)
 */
#define isgreaterequal(x, y)  (!isunordered((x), (y)) && ((x) >= (y)))

/**
 * @brief Returns non-zero if x <= y.
 * @note islessequal(x, y)
 */
#define islessequal(x, y)     (!isunordered((x), (y)) && ((x) <= (y)))

/**
 * @brief Returns non-zero if (x < y) or (x > y).
 * @note islessgreater(x, y)
 * @details Under the !isunordered guard (meaning x and y are valid numbers),
 * the condition "x is not equal to y" is mathematically identical to
 * "x is less than y or x is greater than y" (Trichotomy Law).
 * Using != is cleaner and requires only one comparison.
 */
#define islessgreater(x, y)   (!isunordered((x), (y)) && ((x) != (y)))

/* CONSTANT VALUES */

/*
 * @brief The normal constant for PI
 */
#define M_PI    3.14159265358979323846
#define M_PI_F  3.14159265358979323846f
#define M_PI_L  3.14159265358979323846L

/*
 * @brief The PI/2 Constant
 */
#define M_PI_2    1.5707963267948966    // M_PI / 2
#define M_PI_2_F  1.5707963267948966f
#define M_PI_2_L  1.5707963267948966L

/*
 * @brief The SQRT_2, LOG10_E, LOG10_E and LN_2 constants
 */
#define SQRT_2      1.41421356237309504880 // sqrt(2)
#define LOG10_2     0.30102999566398119521 // log10(2)
#define LOG10_E     0.43429448190325182765 // 1 / ln(10)
#define LN_2        0.69314718055994530941 // ln(2)


/// Private values
#define DBL_EXP_MASK    0x7FF0000000000000ULL
#define DBL_MANT_MASK   0x000FFFFFFFFFFFFFULL
#define DBL_EXP_BIAS    1023
#define DBL_EXP_SHIFT   52

/*
 * @brief A positive float infinity.
 */
#ifndef INFINITY    // +inf
#define INFINITY (1.0f / 0.0f)
#endif

/*
 * @brief A "Quiet" Not-a-Number.
 * @details 0.0f divided by 0.0f produces NaN.
 */
#ifndef NAN
#define NAN (0.0f / 0.0f)
#endif

/*
 * @brief Represents double infinity.
 * @note Historically used as a fallback for overflow before INFINITY existed.
 * We simply cast the float INFINITY to double.
 */
#define HUGE_VAL ((double)INFINITY)

/*
 * @brief The float version of HUGE_VAL.
 */
#define HUGE_VALF (INFINITY)

/*
 * @brief HUGE_VALL
 * @notes The long double version of HUGE_VAL.
 */
#define HUGE_VALL ((long double)INFINITY)

/* ERROR HANDLING */

/*
 * @brief Error handling mode flags for math_errhandling
 *
 * @note MATH_ERRNO indicates errors are reported via 'errno'.
 * @note MATH_ERREXCEPT indicates errors are reported via 'fenv.h' exceptions.
 */
#define MATH_ERRNO     1
#define MATH_ERREXCEPT 2

/*
 * @brief Implements correctly the math_errhandling definition.
 */
#if defined(__STDC_IEC_559__)
    #define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)
#elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 1)
    #define math_errhandling (MATH_ERRNO)
#else
    #define math_errhandling 0
#endif

/* FUNCTIONS */

/**
 * @brief Compute arc cosine.
 * @param x Value whose arc cosine is computed, in the interval [-1, 1].
 * @return Principal arc cosine of x, in the interval [0, pi] radians.
 */
double acos(double x);

/**
 * @brief Compute arc sine.
 * @param x Value whose arc sine is computed, in the interval [-1, 1].
 * @return Principal arc sine of x, in the interval [-pi/2, pi/2] radians.
 */
double asin(double x);

/**
 * @brief Compute arc tangent.
 * @param x Value whose arc tangent is computed.
 * @return Principal arc tangent of x, in the interval [-pi/2, pi/2] radians.
 */
double atan(double x);

/**
 * @brief Compute arc tangent of two variables.
 * @param y Y-component. @param x X-component.
 * @return Arc tangent in radians, in the interval [-pi, pi].
 */
double atan2(double y, double x);

/**
 * @brief Rounds x upward to the nearest integer.
 * @param x Value to round.
 * @return Smallest integer value not less than x.
 */
double ceil(double x);

/**
 * @brief Compute cosine.
 * @param x Angle in radians.
 * @return Cosine of x.
 */
double cos(double x);

/**
 * @brief Compute hyperbolic cosine.
 * @param x Value to compute cosh for.
 * @return Hyperbolic cosine of x.
 */
double cosh(double x);

/**
 * @brief Compute exponential function (e^x).
 * @param x Exponent value.
 * @return Exponential value of x.
 */
double exp(double x);

/**
 * @brief Compute absolute value.
 * @param x Floating point value.
 * @return Absolute value of x.
 */
double fabs(double x);

/**
 * @brief Rounds x downward to the nearest integer.
 * @param x Value to round.
 * @return Largest integer value not greater than x.
 */
double floor(double x);

/**
 * @brief Compute remainder of division.
 * @param x Numerator. @param y Denominator.
 * @return Remainder of x/y.
 */
double fmod(double x, double y);

/**
 * @brief Split value into fraction and exponent (value = frac * 2^exp).
 * @param value The floating point value to decompose.
 * @param exp Pointer to an integer where the exponent is stored.
 * @return Fraction in the range [0.5, 1) or 0.
 */
double frexp(double value, int *exp);

/**
 * @brief Compute x * 2^exp.
 * @param x Value to multiply. @param exp Integer exponent.
 * @return Result of x multiplied by 2 raised to exp.
 */
double ldexp(double x, int exp);

/**
 * @brief Compute natural logarithm (base-e).
 * @param x Value whose logarithm is computed.
 * @return Natural logarithm of x.
 */
double log(double x);

/**
 * @brief Compute common logarithm (base-10).
 * @param x Value whose logarithm is computed.
 * @return Common logarithm of x.
 */
double log10(double x);

/**
 * @brief Compute x raised to the power of y.
 * @param x Base value. @param y Exponent value.
 * @return Result of x^y.
 */
double pow(double x, double y);

/**
 * @brief Compute sine.
 * @param x Angle in radians.
 * @return Sine of x.
 */
double sin(double x);

/**
 * @brief Compute hyperbolic sine.
 * @param x Value to compute sinh for.
 * @return Hyperbolic sine of x.
 */
double sinh(double x);

/**
 * @brief Compute square root.
 * @param x Value to compute sqrt for (non-negative).
 * @return Square root of x.
 */
double sqrt(double x);

/**
 * @brief Compute tangent.
 * @param x Angle in radians.
 * @return Tangent of x.
 */
double tan(double x);

/**
 * @brief Compute hyperbolic tangent.
 * @param x Value to compute tanh for.
 * @return Hyperbolic tangent of x.
 */
double tanh(double x);

#endif
