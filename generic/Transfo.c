/*
 * Transfo.c -- Implementation of transformation routines.
 *
 * Authors		: Patrick Lecoanet.
 * Creation date	: 
 *
 * $Id: Transfo.c,v 1.10 2003/04/16 09:49:22 lecoanet Exp $
 */

/*
 *  Copyright (c) 1993 - 1999 CENA, Patrick Lecoanet --
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this code; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * This package deals with *AFFINE* 3x3 matrices.
 * This means that you should not try to feed it with matrices
 * containing perspective changes. It is assumed that the third
 * column is always [0 0 1] as this is the case for affine matrices.
 * Furthermore affine matrices are known to be invertible (non singular).
 * Despite this, various tests are done to test the invertibility because
 * of numerical precision or limit.
 * Any of the operations in this module yield an affine matrix. Composition
 * of two affine matrices and inversion of an affine matrix result in an
 * affine matrix (Affine matrices Group property). Rotation, translation
 * anamorphic scaling, xy shear and yx shear also preserve the property.
 *
 */


#include "Item.h"
#include "Geo.h"
#include "Transfo.h"
#include "Types.h"

#include <stdlib.h>


static const char rcsid[] = "$Imagine: Transfo.c,v 1.7 1997/01/24 14:33:37 lecoanet Exp $";
static const char compile_id[]="$Compile: " __FILE__ " " __DATE__ " " __TIME__ " $";



/*
 *************************************************************************
 *
 * The transformation primitives are based on affines matrices retricted
 * to the following pattern:
 *
 *	x x 0
 *	x x 0
 *	x x 1
 *
 * It is necessary to only feed such matrices to these primitives as they
 * do optimizations based on their properties. Furthermore the package
 * stores only the first two columns, the third is constant. There is no
 * way to describe perspective transformation with these transformation
 * matrices.
 *
 *************************************************************************
 */

/*
 *************************************************************************
 *
 * ZnTransfoNew --
 *	Create a new transformation and return it initialized to
 *	identity.
 * 
 *************************************************************************
 */
ZnTransfo *
ZnTransfoNew()
{
  ZnTransfo	*t;

  t = (ZnTransfo *) ZnMalloc(sizeof(ZnTransfo));
  ZnTransfoSetIdentity(t);
  
  return t;
}


/*
 *************************************************************************
 *
 * ZnTransfoDuplicate --
 *	Create a new transformation identical to the model t.
 * 
 *************************************************************************
 */
ZnTransfo *
ZnTransfoDuplicate(ZnTransfo *t)
{
  ZnTransfo	*nt;

  nt = (ZnTransfo *) ZnMalloc(sizeof(ZnTransfo));
  if (t) {
    *nt = *t;
  }
  else {
    ZnTransfoSetIdentity(nt);
  }
  
  return nt;
}


/*
 *************************************************************************
 *
 * ZnTransfoFree --
 *	Delete a transformation and free its memory.
 * 
 *************************************************************************
 */
void
ZnTransfoFree(ZnTransfo	*t)
{
  ZnFree(t);
}


/*
 *************************************************************************
 *
 * ZnPrintTransfo --
 *	Print the transfo matrix on stdout.
 * 
 *************************************************************************
 */
void
ZnPrintTransfo(ZnTransfo	*t)
{
  printf("(%5g %5g\n %5g %5g\n %5g %5g)\n",
	 t->_[0][0], t->_[0][1],
	 t->_[1][0], t->_[1][1],
	 t->_[2][0], t->_[2][1]);
}


/*
 *************************************************************************
 *
 * ZnTransfoIsIdentity --
 *	Tell if the given transfo is (close to) identity.
 * 
 *************************************************************************
 */
ZnBool
ZnTransfoIsIdentity(ZnTransfo	*t)
{
  ZnReal	tmp;
  ZnBool	res = False;

  tmp = t->_[0][0] - 1.0;
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  tmp = t->_[1][1] - 1.0;
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  tmp = t->_[0][1];
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  tmp = t->_[1][0];
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  tmp = t->_[2][0];
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  tmp = t->_[2][1];
  res = res & ((tmp < PRECISION_LIMIT) && (tmp > -PRECISION_LIMIT));
  return res;
}


/*
 *************************************************************************
 *
 * ZnTransfoSetIdentity --
 *	Initialize the given transfo to identity.
 * 
 *************************************************************************
 */
void
ZnTransfoSetIdentity(ZnTransfo	*t)
{
  t->_[0][0] = 1;
  t->_[0][1] = 0;
  t->_[1][0] = 0;
  t->_[1][1] = 1;
  t->_[2][0] = 0;
  t->_[2][1] = 0;
}


/*
 *************************************************************************
 *
 * ZnTransfoCompose --
 *	Combine two transformations t1 and t2 by post-concatenation.
 *	Returns the resulting transformation.
 *	t2 can be NULL, meaning identity transform. This is used in
 *	the toolkit to optimize some cases.
 *
 *	All the parameters must be distincts transforms.
 *
 *************************************************************************
 */
ZnTransfo *
ZnTransfoCompose(ZnTransfo	*res,
		 ZnTransfo	*t1,
		 ZnTransfo	*t2)
{
  if ((t1 != NULL) && (t2 != NULL)) {
    register ZnReal	tmp;

    tmp = t1->_[0][0];
    res->_[0][0] = tmp*t2->_[0][0] + t1->_[0][1]*t2->_[1][0];
    res->_[0][1] = tmp*t2->_[0][1] + t1->_[0][1]*t2->_[1][1];
    tmp = t1->_[1][0];
    res->_[1][0] = tmp*t2->_[0][0] + t1->_[1][1]*t2->_[1][0];
    res->_[1][1] = tmp*t2->_[0][1] + t1->_[1][1]*t2->_[1][1];
    tmp = t1->_[2][0];
    res->_[2][0] = tmp*t2->_[0][0] + t1->_[2][1]*t2->_[1][0] + t2->_[2][0];
    res->_[2][1] = tmp*t2->_[0][1] + t1->_[2][1]*t2->_[1][1] + t2->_[2][1];
  }
  else if (t1 == NULL) {
    if (res != t2) {
      *res = *t2;
    }
  }
  else if (t2 == NULL) {
    if (res != t2) {
      *res = *t1;
    }
  }
  else {
    ZnTransfoSetIdentity(res);
  }
  
  return res;
}


/*
 *************************************************************************
 *
 * ZnTransfoInvert --
 *	Compute the inverse of the given matrix and return it. This
 *	function makes the assumption that the matrix is affine to
 *	optimize the job. Do not give it a general matrix, this will
 *	fail. This code is from Graphics Gems II. Anyway an affine
 *	matrix is always invertible for affine matrices form a sub
 *	group of the non-singular matrices.
 *
 *************************************************************************
 */
ZnTransfo *
ZnTransfoInvert(ZnTransfo	*t,
		ZnTransfo	*inv)
{
  ZnReal	pos, neg, temp, det_l;

  if (t == NULL) {
    return NULL;
  }

  /*
   * Compute the determinant of the upper left 2x2 sub matrix to see
   * if it is singular.
   */
  pos = neg = 0.0;
  temp = t->_[0][0] * t->_[1][1];
  if (temp >= 0.0) {
    pos += temp;
  }
  else {
    neg += temp;
  }
  temp = - t->_[0][1] * t->_[1][0];
  if (temp >= 0.0) {
    pos += temp;
  }
  else {
    neg += temp;
  }
  det_l = pos + neg;
  temp = det_l / (pos - neg); /* Why divide by (pos - neg) ?? */
  
  if (ABS(temp) < PRECISION_LIMIT) {
    ZnWarning("ZnTransfoInvert : singular matrix\n");
    return NULL;
  }
  
  det_l = 1.0/ det_l;
  inv->_[0][0] = t->_[1][1] * det_l;
  inv->_[0][1] = - t->_[0][1] * det_l;
  inv->_[1][0] = - t->_[1][0] * det_l;
  inv->_[1][1] = t->_[0][0] * det_l;
  /*
   * The code below is equivalent to:
   *   inv->_[2][0] = (t->_[1][0] * t->_[2][1] - t->_[1][1] * t->_[2][0]) * det_l;
   *   inv->_[2][1] = - (t->_[0][0] * t->_[2][1] - t->_[0][1] * t->_[2][0]) * det_l;
   *
   * with some operations factored (already computed) to increase speed.
   */
  inv->_[2][0] = - (inv->_[0][0] * t->_[2][0] + inv->_[1][0] * t->_[2][1]);
  inv->_[2][1] = - (inv->_[0][1] * t->_[2][0] + inv->_[1][1] * t->_[2][1]);

  return inv;
}


/*
 *************************************************************************
 *
 * ZnTransfoDecompose --
 *	Decompose an affine matrix into translation, scale, shear and
 *	rotation. The different values are stored in the locations
 *	pointed to by the pointer parameters. If some values are not
 *	needed a NULL pointer can be given instead. The resulting shear
 *	shears x coordinate when y change.
 *	This code is taken from Graphics Gems II.
 *
 *************************************************************************
 */
void
ZnTransfoDecompose(ZnTransfo	*t,
		   ZnPoint	*scale,
		   ZnPoint	*trans,
		   ZnReal	*rotation,
		   ZnReal	*shearxy)
{
  ZnTransfo	local;
  ZnReal	shear, len, rot, det;
  
  if (t == NULL) {
    /* Identity transform */
    if (scale) {
      scale->x = 1.0;
      scale->y = 1.0;
    }
    if (trans) {
      trans->x = 0.0;
      trans->y = 0.0;
    }
    if (rotation) {
      *rotation = 0.0;
    }
    if (shearxy) {
      *shearxy = 0.0;
    }
    //printf("Transfo is identity\n");
    return;
  }

  det = (t->_[0][0]*t->_[1][1] - t->_[0][1]*t->_[1][0]);
  if (ABS(det) < PRECISION_LIMIT) {
    ZnWarning("ZnTransfoDecompose : singular matrix\n");
    return;
  }
  
  local = *t;
  //ZnPrintTransfo(&local);
  /* Get translation part if needed */
  if (trans) {
    trans->x = ABS(local._[2][0]) < PRECISION_LIMIT ? 0 : local._[2][0];
    trans->y = ABS(local._[2][1]) < PRECISION_LIMIT ? 0 : local._[2][1];
  }
  if (!scale && !shearxy && !rotation) {
    return;
  }

  /* Get scale and shear */
  len = sqrt(local._[0][0]*local._[0][0] +
	     local._[0][1]*local._[0][1]); /* Get x scale from 1st row */
  if (scale) {
    scale->x = len < PRECISION_LIMIT ? 0.0 : len;
  }
  local._[0][0] /= len;			 /* Normalize 1st row */
  local._[0][1] /= len;
  shear = (local._[0][0]*local._[1][0] +
	   local._[0][1]*local._[1][1]); /* Shear is dot product of 1st row & 2nd row */
  /* Make the 2nd row orthogonal to the 1st row
   * by linear combinaison:
   * row1.x = row1.x + row0.x*-shear &
   * row1.y = row1.y + row0.y*-shear
   */
  local._[1][0] -= local._[0][0]*shear;
  local._[1][1] -= local._[0][1]*shear;
  len = sqrt(local._[1][0]*local._[1][0] +
	     local._[1][1]*local._[1][1]); /* Get y scale from 2nd row */
  if (scale) {
    scale->y = len < PRECISION_LIMIT ? 0.0 : len;
  }

  if (!shearxy && !rotation) {
    return;
  }

  local._[1][0] /= len;			 /* Normalize 2nd row */
  local._[1][1] /= len;
  shear /= len;
  if (shearxy) {
    *shearxy = ABS(shear) < PRECISION_LIMIT ? 0.0 : shear;
    //printf("shear %f\n", *shearxy);
  }

  if (!rotation) {
    return;
  }

  //printf("Matrix after scale & shear extracted\n");
  //ZnPrintTransfo(&local);
  /* Get rotation */
  /* Check for a coordinate system flip. If det of upper-left 2x2
   * is -1, there is a reflection. If the rotation is < 180� negate
   * the y scale. If the rotation is > 180� then negate the x scale
   * and report a rotation between 0 and 180�. This dissymetry is
   * the result of computing (z) rotation from the first row (x component
   * of the axis system basis).
   */
  det = (local._[0][0]*local._[1][1]- local._[0][1]*local._[1][0]);
  
  rot = atan2(local._[0][1], local._[0][0]);
  if (rot < 0) {
    rot = 2*M_PI+rot;
  }
  rot = rot < PRECISION_LIMIT ? 0.0 : rot;
  if (rot >= M_PI) {
    /*rot -= M_PI;  Why that, I'll have to check Graphic Gems ??? */
    if (scale && det < 0) {
      scale->x *= -1;
    }
  }
  else if (scale && det < 0) {
    scale->y *= -1;
  }
  
  //printf("scalex %f\n", scale->x);
  //printf("scaley %f\n", scale->y);
  //printf("rotation %f\n", rot*180.0/3.1415);

  if (rotation) {
    *rotation = rot;
  }
}


/*
 *************************************************************************
 *
 * ZnTransfoEqual --
 *	Return True if t1 and t2 are equal (i.e they have the same
 *	rotation, shear scales and translations). If include_translation
 *	is True the translations are considered in the test.
 *
 *************************************************************************
 */
ZnBool
ZnTransfoEqual(ZnTransfo	*t1,
	       ZnTransfo	*t2,
	       ZnBool		include_translation)
{
  if (include_translation) {
    return (t1->_[0][0] == t2->_[0][0] &&
	    t1->_[0][1] == t2->_[0][1] &&
	    t1->_[1][0] == t2->_[1][0] &&
	    t1->_[1][1] == t2->_[1][1] &&
	    t1->_[2][0] == t2->_[2][0] &&
	    t1->_[2][1] == t2->_[2][1]);
  }
  else {
    return (t1->_[0][0] == t2->_[0][0] &&
	    t1->_[0][1] == t2->_[0][1] &&
	    t1->_[1][0] == t2->_[1][0] &&
	    t1->_[1][1] == t2->_[1][1]);
  }
}


/*
 *************************************************************************
 *
 * ZnTransfoHasShear --
 *	Return True if t has a shear factor in x or y or describe a
 *	rotation or both.
 *
 *************************************************************************
 */
ZnBool
ZnTransfoHasShear(ZnTransfo	*t)
{
  return t->_[0][1] != 0.0 || t->_[1][0] != 0.0;
}


/*
 *************************************************************************
 *
 * ZnTransfoIsTranslation --
 *	Return True if t is a pure translation.
 *
 *************************************************************************
 */
ZnBool
ZnTransfoIsTranslation(ZnTransfo	*t)
{
  return (t->_[0][0] == 0.0 &&
	  t->_[0][1] == 0.0 &&
	  t->_[1][0] == 0.0 &&
	  t->_[1][1] == 0.0);
}


/*
 *************************************************************************
 *
 * ZnTransformPoint --
 *	Apply the transformation to the point. The point is
 *	modified and returned as the value of the function.
 *	A NULL transformation means identity. This is only used
 *	in the toolkit to optimize some cases. It should never
 *	happen in user code.
 *
 *************************************************************************
 */
ZnPoint *
ZnTransformPoint(ZnTransfo		*t,
		 register ZnPoint	*p,
		 ZnPoint		*xp)
{
  if (t == NULL) {
    xp->x = p->x;
    xp->y = p->y;
  }
  else {
    xp->x = t->_[0][0]*p->x + t->_[1][0]*p->y + t->_[2][0];
    xp->y = t->_[0][1]*p->x + t->_[1][1]*p->y + t->_[2][1];
  }
  return xp;
}


/*
 *************************************************************************
 *
 * ZnTransformPoints --
 *	Apply the transformation to the points in p returning points in xp.
 *	The number of points is in num.
 *	A NULL transformation means identity. This is only used
 *	in the toolkit to optimize some cases. It should never
 *	happen in user code.
 *
 *************************************************************************
 */
void
ZnTransformPoints(ZnTransfo	*t,
		  ZnPoint	*p,
		  ZnPoint	*xp,
		  unsigned int	num)
{
  if (t == NULL) {
    memcpy(xp, p, sizeof(ZnPoint)*num);
  }
  else {
    unsigned int i;
    
    for (i = 0; i < num; i++) {
      xp[i].x = t->_[0][0]*p[i].x + t->_[1][0]*p[i].y + t->_[2][0];
      xp[i].y = t->_[0][1]*p[i].x + t->_[1][1]*p[i].y + t->_[2][1];
    }
  }
}


/*
 *************************************************************************
 *
 * ZnTranslate --
 *	Translate the given transformation by delta_x, delta_y. Returns
 *	the resulting transformation.
 *
 * ZnSetTranslation --
 *	Set the translation instead of combining it into the
 *	transformation.
 *
 *************************************************************************
 */
ZnTransfo *
ZnTranslate(ZnTransfo	*t,
	    ZnReal	delta_x,
	    ZnReal	delta_y)
{
  t->_[2][0] = t->_[2][0] + delta_x;
  t->_[2][1] = t->_[2][1] + delta_y;

  return t;
}

ZnTransfo *
ZnSetTranslation(ZnTransfo	*t,
		 ZnReal		delta_x,
		 ZnReal		delta_y)
{
  t->_[2][0] = delta_x;
  t->_[2][1] = delta_y;

  return t;
}


/*
 *************************************************************************
 *
 * ZnScale --
 *	Scale the given transformation by scale_x, scale_y. Returns the
 *	resulting transformation.
 *
 *************************************************************************
 */
ZnTransfo *
ZnScale(ZnTransfo	*t,
	register ZnReal	scale_x,
	register ZnReal	scale_y)
{
  t->_[0][0] = t->_[0][0]*scale_x;
  t->_[0][1] = t->_[0][1]*scale_y;
  t->_[1][0] = t->_[1][0]*scale_x;
  t->_[1][1] = t->_[1][1]*scale_y;
  t->_[2][0] = t->_[2][0]*scale_x;
  t->_[2][1] = t->_[2][1]*scale_y;
  
  return t;
}


/*
 *************************************************************************
 *
 * ZnRotateRad --
 *	Rotate the given transformation by angle radians
 *	counter-clockwise around the origin. Returns the resulting
 *	transformation.
 *
 *************************************************************************
 */
ZnTransfo *
ZnRotateRad(ZnTransfo	*t,
	    ZnReal	angle)
{
  register ZnReal	c = cos(angle);
  register ZnReal	s = sin(angle);
  register ZnReal	tmp;

  tmp = t->_[0][0];
  t->_[0][0] = tmp*c - t->_[0][1]*s;
  t->_[0][1] = tmp*s + t->_[0][1]*c;
  tmp = t->_[1][0];
  t->_[1][0] = tmp*c - t->_[1][1]*s;
  t->_[1][1] = tmp*s + t->_[1][1]*c;
  tmp = t->_[2][0];
  t->_[2][0] = tmp*c - t->_[2][1]*s;
  t->_[2][1] = tmp*s + t->_[2][1]*c;
  
  return t;
}


/*
 *************************************************************************
 *
 * ZnRotateDeg --
 *	Rotate the given transformation by angle degrees
 *	counter-clockwise around the origin. Returns the resulting
 *	transformation.
 *
 *************************************************************************
 */
ZnTransfo *
ZnRotateDeg(ZnTransfo	*t,
	    ZnReal	angle)
{
  return ZnRotateRad(t, ZnDegRad(angle));
}

  
#undef PRECISION_LIMIT


