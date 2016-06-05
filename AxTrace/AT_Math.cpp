#include "stdafx.h"
#include "AT_Math.h"

namespace AT3
{

//-------------------------------------------------------------------------------------------------------------------
/** 浮点数比较
@return
-1 : a<b
0 : a==b
1 : a>b
*/
int floatCompare(Real a, Real b, Real tolerance)
{
	Real c = a - b;

	if (c > tolerance) return 1;
	else if (c < -tolerance) return -1;
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
const fVector2 fVector2::ZERO(0, 0);

//-------------------------------------------------------------------------------------------------------------------
Real fVector2::length(void) const 
{
	return sqrt(x*x+y*y);
}

//-------------------------------------------------------------------------------------------------------------------
Real fVector2::normalize(void)
{
	Real len = sqrt(x*x+y*y);
	if(floatCompare(len, 0)<=0) return 0.0;

	Real inv_len = 1.0f/len;

	x *= inv_len;
	y *= inv_len;

	return len;
}

}
