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
    ptvEvaluatedCurvePts.clear();

    //
    // Interpolate the end points by adding a copy of the first and last points
    // to the control point vector.
    //
    std::vector<Point> interpolatedPoints;
    std::vector<Point>::iterator it;

    interpolatedPoints.push_back(ptvCtrlPts.front());
    it = interpolatedPoints.begin();
    interpolatedPoints.insert(it+1, ptvCtrlPts.begin(), ptvCtrlPts.end());
    interpolatedPoints.push_back(ptvCtrlPts.back());

    //
    // Add the first point from the list of control points
    // so the animation doesn't have a gap at the beginning.
    //
    float x = 0.0;
    float y1 = interpolatedPoints[0].y;
    ptvEvaluatedCurvePts.push_back(Point(x, y1));


    int iCtrlPtCount = interpolatedPoints.size();

    //
    // Add a point equal to the final point in the animation
    // so the animation doesn't have a gap at the end.
    //
    float y2 = interpolatedPoints[iCtrlPtCount - 1].y;;
    x = fAniLength;
    ptvEvaluatedCurvePts.push_back(Point(x, y2));


    //
    // Loop through control points in chunks of 4, incrementing by 1
    // and compute first the Catmull-Rom control points and then the
    // Bezier curve between these points.
    //
    int i = 3;
    while(i < iCtrlPtCount)
    {
        Vec3d V0 = Vec3d(interpolatedPoints[i-2].x, interpolatedPoints[i-2].y, 1);      //P1
        Vec3d V3 = Vec3d(interpolatedPoints[i-1].x, interpolatedPoints[i-1].y, 1);      //P2

        Point P0 = Point(interpolatedPoints[i-3].x, interpolatedPoints[i-3].y);
        Point P1 = Point(interpolatedPoints[i-2].x, interpolatedPoints[i-2].y);
        Point P2 = Point(interpolatedPoints[i-1].x, interpolatedPoints[i-1].y);
        Point P3 = Point(interpolatedPoints[i].x, interpolatedPoints[i].y);

        //
        // Compute V1
        //
        Point PV1 = Point(P2.x - P0.x, P2.y - P0.y);
        PV1.x = (PV1.x/6);
        PV1.y = (PV1.y/6);
        PV1.x += P1.x;
        PV1.y += P1.y;

        //
        // Compute V2
        //
        Point PV2 = Point(P3.x - P1.x, P3.y - P1.y);
        PV2.x = -(PV2.x/6);
        PV2.y = -(PV2.y/6);
        PV2.x += P2.x;
        PV2.y += P2.y;


        Vec3d V1 = Vec3d(PV1.x, PV1.y, 1);
        Vec3d V2 = Vec3d(PV2.x, PV2.y, 1);


        float prev_x = 0.0;
        for(float u = 0.0; u < 1.0; u+=0.04)
        {
            //
            // Solve for Q(u)
            //
            Vec3d Qu = (((1 - u)*(1 - u)*(1 - u))*V0) +
                ((3*u)*((1-u)*(1-u))*V1) +
                ((3*(u*u))*(1-u)*V2) +
                ((u*u*u)*V3);

            //
            // Ensure curve is a continuous function of time by
            // checking that x is increasing monotonically.
            //
            if(Qu[0] > prev_x)
            {
                prev_x = Qu[0];

                //
                // Insure Qu.x is less than P2
                //
                if(Qu[0] > P2.x)
                {
                    Qu[0] = P2.x;
                    Qu[1] = P2.y;
                }

                //
                // Insure Qu.x > P1
                //
                if(Qu[0] < P1.x)
                {
                    Qu[0] = P1.x;
                    Qu[1] = P1.y;
                }

                ptvEvaluatedCurvePts.push_back(Point(Qu[0], Qu[1]));
            }
        }

        //
        // Add the final point to prevent discontinuity for overlapping curves
        //
        ptvEvaluatedCurvePts.push_back(Point(V3[0], V3[1]));

        i++;
    }
}
