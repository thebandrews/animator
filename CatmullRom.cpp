#include "catmullrom.h"
#include "vec.h"
#include <cassert>

using namespace Animator;

void CatmullRomCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts, 
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
    int i = 2;
    int j = 0;
    while((i+1) < iCtrlPtCount)
    {
        j = i;
        //
        // Save P0 as part of the curve
        //
        ptvEvaluatedCurvePts.push_back(Point(ptvCtrlPts[i-2].x, ptvCtrlPts[i-2].y));


        //
        // Save our control points as vectors
        //
        Vec3d V0 = Vec3d(ptvCtrlPts[i-1].x, ptvCtrlPts[i-1].y, 1);      //P1
        Vec3d V3 = Vec3d(ptvCtrlPts[i].x, ptvCtrlPts[i].y, 1);      //P2

        //
        // Add V0, V3 to evaluated curve
        //
        ptvEvaluatedCurvePts.push_back(Point(V0[0], V0[1]));
        ptvEvaluatedCurvePts.push_back(Point(V3[0], V3[1]));

        //
        // Compute V1, V2
        //
        Point P0 = Point(ptvCtrlPts[i-2].x, ptvCtrlPts[i-2].y);
        Point P1 = Point(ptvCtrlPts[i-1].x, ptvCtrlPts[i-1].y);
        Point P2 = Point(ptvCtrlPts[i].x, ptvCtrlPts[i].y);
        Point P3 = Point(ptvCtrlPts[i+1].x, ptvCtrlPts[i+1].y);

        Point PV1 = P1 + (P2-P0)*(1/6);
        Point PV2 = P2 - (P3-P1)*(1/6);

        Vec3d V1 = Vec3d(PV1.x, PV1.y, 1);
        Vec3d V2 = Vec3d(PV2.x, PV2.y, 1);

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
        i+=2;
    }


    //
    // Do we have any dangling control points? Handle them here
    // TODO: Do we need to interpolate end points? Maybe.
    //
    //if(j > 0){
    //    // 
    //    // If j > 0 this means we have already processed some
    //    // points and incremented j in intervals of 3 so go to
    //    // next point. If j == 0 then we don't have enough
    //    // points to compute a bezier curve so just copy
    //    // the points from the control vector.
    //    //
    //    j++;
    //}

    //while(j < iCtrlPtCount)
    //{
    //    ptvEvaluatedCurvePts.push_back(Point(ptvCtrlPts[j].x, ptvCtrlPts[j].y));
    //    j++;
    //}

}
