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
 * This code may be used and freely distributed as long as it includes
 * this copyright notice and the above warranty information.
 */

//Modified by Till Rosenband, 9/16/2009

#ifndef CORDIC_H
#define CORDIC_H

/* [TR] Return angle of x,y cartesion coords.
   [TR] Angle units are set up for 16 bit precision.
   [TR] 1 LSB = 2*pi*2^-16. (BRAD16)
   [TR] Tested from -pi ... pi.  Standard error = 1.15 LSB. Max error = 2.9 LSB.
   [TR] Inputs should be about 2^27 for maximum precision
 */

int FxAtan2S(short x, short y);
int FxAtan2(int x, int y);
void FxPolarize(int *argx, int *argy);

/* Fxrotate() rotates the vector (argx, argy) by the angle theta. */
void FxRotate(int *argx, int *argy, int theta);

/* Return a unit vector corresponding to theta.
 * sin and cos are fixed-point fractions.
 */
void FxUnitVec(int *cos, int *sin, int theta);


#endif //CORDIC_H
