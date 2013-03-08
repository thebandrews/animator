#ifndef INCLUDED_CATMULL_ROM_H
#define INCLUDED_CATMULL_ROM_H

#pragma warning(disable : 4786)  
#pragma warning(disable : 4018)
#pragma warning(disable : 4267)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)

#include "curveevaluator.h"

class CatmullRomCurveEvaluator : public CurveEvaluator
{
public:
        void evaluateCurve(const std::vector<Animator::Point>& ptvCtrlPts,
                std::vector<Animator::Point>& ptvEvaluatedCurvePts,
        const float& fAniLength, 
        const bool& bWrap,
        const float default_val) const;
};

#endif
