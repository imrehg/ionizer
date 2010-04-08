/* Copyright (C) 1981-1999 Ken Turkowski.
 *
 * All rights reserved.
 *
 * Warranty Information
 *  Even though I have reviewed this software, I make no warranty
 *  or representation, either express or implied, with respect to this
 *  software, its quality, accuracy, merchantability, or fitness for a
 *  particular purpose.  As a result, this software is provided "as is,"
 *  and you, its user, are assuming the entire risk as to its quality
 *  and accuracy.
 *
 * This code may be used and freely distributed as int as it includes
 * this copyright notice and the above warranty information.
 */

//Modified by Till Rosenband, 9/16/2009

/*#define FASTER*/
/*#define DEGREES 1*/


#define NOT_IN_ASSEMBLER
#define BRAD16 1
#define COSCALE 0x11616E8E /* 291597966 = 0.2715717684432241 * 2^30, valid for j>13 */

/* TR: for the chosen angle units, arctantab = {atan(4), atan(2), ..., 1} */
static int arctantab[] = {

#ifdef DEGREES      /* 16 fractional bits */
# define QUARTER (90L << 16)
# define MAXITER 22 /* the resolution of the arctan table */
    4157273, 2949120, 1740967, 919879, 466945, 234379, 117304, 58666,
    29335, 14668, 7334, 3667, 1833, 917, 458, 229,
    115, 57, 29, 14, 7, 4, 2, 1
#else /* !DEGREES */

# ifdef RADIANS /* 16 fractional bits */
#  define QUARTER ((int)(3.141592654 / 2.0 * (1L << 16)))
#  define MAXITER 16    /* the resolution of the arctan table */
    72558, 51472, 30386, 16055, 8150, 4091, 2047, 1024,
    512, 256, 128, 64, 32, 16, 8, 4, 2, 1
#  else /* !RADIANS && !DEGREES */

# ifdef BRAD16 /* 2^16 <=> 2pi, 1 LSB = 9.58738e-005 radians*/
#  define QUARTER (16384)
#  define MAXITER 13    /* the resolution of the arctan table */
    11548, 8192, 4836, 2555, 1297, 651, 326, 163, 81, 41, 20, 10, 5, 3, 1

#  else /* !RADIANS && !DEGREES && !BRAD16*/

#  define BRADS 1
#  define QUARTER (1L << 30)
#  define MAXITER 16    /* the resolution of the arctan table */
    756808418, 536870912, 316933406, 167458907, 85004756, 42667331,
    21354465, 10679838, 5340245, 2670163, 1335087, 667544, 333772, 166886,
    83443, 41722, 20861, 10430, 5215, 2608, 1304, 652, 326, 163, 81, 41,
    20, 10, 5, 3, 1
# endif /* !RADIANS && !DEGREES && !BRAD16*/
#endif
#endif /* !DEGREES */

};


/* To rotate a vector through an angle of theta, we calculate:
 *
 *  x' = x cos(theta) - y sin(theta)
 *  y' = x sin(theta) + y cos(theta)
 *
 * The rotate() routine performs multiple rotations of the form:
 *
 *  x[i+1] = cos(theta[i]) { x[i] - y[i] tan(theta[i]) }
 *  y[i+1] = cos(theta[i]) { y[i] + x[i] tan(theta[i]) }
 *
 * with the constraint that tan(theta[i]) = pow(2, -i), which can be
 * implemented by shifting. We always shift by either a positive or
 * negative angle, so the convergence has the ringing property. Since the
 * cosine is always positive for positive and negative angles between -90
 * and 90 degrees, a predictable magnitude scaling occurs at each step,
 * and can be compensated for instead at the end of the iterations by a
 * composite scaling of the product of all the cos(theta[i])'s.
 */


typedef int TFract;    /* f * 2^30 */
TFract FFracMul(TFract a, TFract b);


static void
PseudoRotate(int *px, int *py, register int theta)
{
    register int i;
    register int x, y, xtemp;
    register int *arctanptr;

    x = *px;
    y = *py;

    /* Get angle between -90 and 90 degrees */
    while (theta < -QUARTER) {
        x = -x;
        y = -y;
        theta += 2 * QUARTER;
    }
    while (theta > QUARTER) {
        x = -x;
        y = -y;
        theta -= 2 * QUARTER;
    }

    /* Initial pseudorotation, with left shift */
    arctanptr = arctantab;
    if (theta < 0) {
        xtemp = x + (y << 1);
        y     = y - (x << 1);
        x     = xtemp;
        theta += *arctanptr++;
    }
    else {
        xtemp = x - (y << 1);
        y     = y + (x << 1);
        x     = xtemp;
        theta -= *arctanptr++;
    }

    /* Subsequent pseudorotations, with right shifts */
    for (i = 0; i <= MAXITER; i++) {
        if (theta < 0) {
            xtemp = x + (y >> i);
            y     = y - (x >> i);
            x     = xtemp;
            theta += *arctanptr++;
        }
        else {
            xtemp = x - (y >> i);
            y     = y + (x >> i);
            x     = xtemp;
            theta -= *arctanptr++;
        }
    }

    *px = x;
    *py = y;
}


static void
PseudoPolarize(int *argx, int *argy)
{
    register int theta;
    register int yi, i;
    register int x, y;
    register int *arctanptr;

    x = *argx;
    y = *argy;

    /* Get the vector into the right half plane */
    theta = 0;
    if (x < 0) {
        x = -x;
        y = -y;
        theta = 2 * QUARTER;
    }

    if (y > 0)
        theta = - theta;

    arctanptr = arctantab+1;

   //Don't think this first step is needed - TR.  See Volder's 1959 paper.
   //
   //if (y < 0) {    /* Rotate positive */
   //     yi = y + (x << 1);
   //     x  = x - (y << 1);
   //     y  = yi;
   //     theta -= *arctanptr++;  /* Subtract angle */
   // }
   // else {      /* Rotate negative */
   //     yi = y - (x << 1);
   //     x  = x + (y << 1);
   //     y  = yi;
   //     theta += *arctanptr++;  /* Add angle */
   // }

    for (i = 0; i <= MAXITER; i++) {
        if (y < 0) {    /* Rotate positive */
            yi = y + (x >> i);
            x  = x - (y >> i);
            y  = yi;
            theta -= *arctanptr++;
        }
        else {      /* Rotate negative */
            yi = y - (x >> i);
            x  = x + (y >> i);
            y  = yi;
            theta += *arctanptr++;
        }
    }

    *argx = x;
    *argy = theta;
}


#ifndef FASTER
/* FxPreNorm() block normalizes the arguments to a magnitude suitable for
 * CORDIC pseudorotations.  The returned value is the block exponent.
 */
static int
FxPreNorm(int *argx, int *argy)
{
    register int x, y;
    int signx, signy;
    register int shiftexp;

    shiftexp = 0;       /* Block normalization exponent */
    signx = signy = 1;

    if ((x = *argx) < 0) {
        x = -x;
        signx = -signx;
    }
    if ((y = *argy) < 0) {
        y = -y;
        signy = -signy;
    }
    /* Prenormalize vector for maximum precision */
    if (x < y) {    /* |y| > |x| */
        while (y < (1 << 27)) {
            x <<= 1;
            y <<= 1;
            shiftexp--;
        }
        while (y > (1 << 28)) {
            x >>= 1;
            y >>= 1;
            shiftexp++;
        }
    }
    else {      /* |x| > |y| */
        while (x < (1 << 27)) {
            x <<= 1;
            y <<= 1;
            shiftexp--;
        }
        while (x > (1 << 28)) {
            x >>= 1;
            y >>= 1;
            shiftexp++;
        }
    }

    *argx = (signx < 0) ? -x : x;
    *argy = (signy < 0) ? -y : y;
    return(shiftexp);
}
#endif //FASTER


/* Return a unit vector corresponding to theta.
 * sin and cos are fixed-point fractions.
 */
void
FxUnitVec(int *cos, int *sin, int theta)
{
    *cos = COSCALE;
    *sin = 0;
    PseudoRotate(cos, sin, theta);
}


/* Fxrotate() rotates the vector (argx, argy) by the angle theta. */
void
FxRotate(int *argx, int *argy, int theta)
{
#ifndef FASTER
    int shiftexp;
#endif //FASTER

    if (((*argx == 0) && (*argy == 0)) || (theta == 0))
        return;

#ifndef FASTER
    shiftexp = FxPreNorm(argx, argy);  /* Prenormalize vector */
#endif //FASTER
    PseudoRotate(argx, argy, theta);   /* Perform CORDIC pseudorotation */
    *argx = FFracMul(*argx, COSCALE);   /* Compensate for CORDIC enlargement */
    *argy = FFracMul(*argy, COSCALE);
#ifndef FASTER
    if (shiftexp < 0) {     /* Denormalize vector */
        *argx >>= -shiftexp;
        *argy >>= -shiftexp;
    }
    else {
        *argx <<= shiftexp;
        *argy <<= shiftexp;
    }
#endif //FASTER
}


int FxAtan2(int x, int y)
{
    if ((x == 0) && (y == 0))
        return(0);
#ifndef FASTER
    FxPreNorm(&x, &y);  /* Prenormalize vector for maximum precision */
#endif //FASTER
    PseudoPolarize(&x, &y); /* Convert to polar coordinates */
    return(y);
}


void
FxPolarize(int *argx, int *argy)
{
#ifndef FASTER
    int shiftexp;
#endif //FASTER

    if ((*argx == 0) && (*argy == 0)) {
        return;
    }

#ifndef FASTER
    /* Prenormalize vector for maximum precision */
    shiftexp = FxPreNorm(argx, argy);
#endif //FASTER

    /* Perform CORDIC conversion to polar coordinates */
    PseudoPolarize(argx, argy);

    /* Scale radius to undo pseudorotation enlargement factor */
    *argx = FFracMul(*argx, COSCALE);

#ifndef FASTER
    /* Denormalize radius */
    *argx = (shiftexp < 0) ? (*argx >> -shiftexp) : (*argx << shiftexp);
#endif //FASTER
}


#ifdef NOT_IN_ASSEMBLER
int
FFracMul(int a, int b)        /* Just for testing */
{
    /* This routine should be written in assembler to calculate the
     * high part of the product, i.e. the product when the operands
     * are considered fractions.
     */
    return((a >> 15) * (b >> 15));
}
#endif //NOT_IN_ASSEMBLER




