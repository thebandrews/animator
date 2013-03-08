#include "beziercurveevaluator.h"
#include "vec.h"
#include <cassert>

using namespace Animator;

void BezierCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts, 
                                         std::vector<Point>& ptvEvaluatedCurvePts, 
                                         const float& fAniLength, 
                                         const bool& bWrap,
                                         const float default_val) const
{
    int iCtrlPtCount = ptvCtrlPts.size();
    ptvEvaluatedCurvePts.clear();

    float x = 0.0;
    float y1 = ptvCtrlPts[0].y;

    ptvEvaluatedCurvePts.push_back(Point(x, y1));

    float y2;
    x = fAniLength;

    y2 = ptvCtrlPts[iCtrlPtCount - 1].y;

    ptvEvaluatedCurvePts.push_back(Point(x, y2));

    //
    // Loop through control points in chunks of 4 where the previous 
    // set of 4 points last point == first point for next set.
    //
    // In order to compute a bezier curve we need at least 4
    // control points. Any dangling control points or curves
    // that have fewer than 4 control points get copied over to
    // the evaluated curve points vector.
    //
    int i = 3;
    int j = 0;
    while(i < iCtrlPtCount)
    {
        j = i;
        //
        // Save our control points as vectors
        //
        Vec3d V0 = Vec3d(ptvCtrlPts[i-3].x, ptvCtrlPts[i-3].y, 1);
        Vec3d V1 = Vec3d(ptvCtrlPts[i-2].x, ptvCtrlPts[i-2].y, 1);
        Vec3d V2 = Vec3d(ptvCtrlPts[i-1].x, ptvCtrlPts[i-1].y, 1);
        Vec3d V3 = Vec3d(ptvCtrlPts[i].x, ptvCtrlPts[i].y, 1);

        //
        // Add the first and last control point in the group of 4
        //
        ptvEvaluatedCurvePts.push_back(Point(V0[0], V0[1]));
        ptvEvaluatedCurvePts.push_back(Point(V3[0], V3[1]));

        //
        // Vary u between 0-1 (100 subdivisions)
        //
        for(float u = 0.0; u < 1.0; u+=0.01)
        {
            //
            // Solve for Q(u)
            //
            Vec3d Qu = (((1 - u)*(1 - u)*(1 - u))*V0) +
                ((3*u)*((1-u)*(1-u))*V1) +
                ((3*(u*u))*(1-u)*V2) +
                ((u*u*u)*V3);


            ptvEvaluatedCurvePts.push_back(Point(Qu[0], Qu[1]));
        }

        // Increment counter
        i+=3;
    }


    //
    // Do we have any dangling control points? Handle them here
    //
    if(j > 0){
        // 
        // If j > 0 this means we have already processed some
        // points and incremented j in intervals of 3 so go to
        // next point. If j == 0 then we don't have enough
        // points to compute a bezier curve so just copy
        // the points from the control vector.
        //
        j++;
    }

    while(j < iCtrlPtCount)
    {
        ptvEvaluatedCurvePts.push_back(Point(ptvCtrlPts[j].x, ptvCtrlPts[j].y));
        j++;
    }

}
