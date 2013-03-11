#pragma warning(disable : 4786)

#include "particleSystem.h"
#include "modelerui.h"


#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
//#include <limits>

using namespace std;

static float prevT;

/***************
* Constructors
***************/

ParticleSystem::ParticleSystem() : 
    restitution("Restitution", 0.0f, 2.0f, 1.0f, 0.1f)
{
    particle_pos = NULL;
    particle_start = NULL;

    // Leave these here; the UI needs them to work correctly.
    dirty = false;
    simulate = false;
}




/*************
* Destructor
*************/

ParticleSystem::~ParticleSystem() 
{
    // TODO
    delete particle_pos;
    delete particle_start;
}


/******************
* Simulation fxns
******************/

/** Start the simulation */
void ParticleSystem::startSimulation(float t)
{
    // YOUR CODE HERE

    // These values are used by the UI ...
    // negative bake_end_time indicates that simulation
    // is still progressing, and allows the
    // indicator window above the time slider
    // to correctly show the "baked" region
    // in grey.
    bake_end_time = -1;
    simulate = true;
    dirty = true;

}

/** Stop the simulation */
void ParticleSystem::stopSimulation(float t)
{
    // YOUR CODE HERE

    // These values are used by the UI
    simulate = false;
    dirty = true;

}

/** Reset the simulation */
void ParticleSystem::resetSimulation(float t)
{
    // YOUR CODE HERE

    // These values are used by the UI
    simulate = false;
    dirty = true;

}

/** Compute forces and update particles **/
void ParticleSystem::computeForcesAndUpdateParticles(float t)
{
    if(simulate )
    {
        //
        // Only compute forces if time updates.
        //
        if(prevT != t)
        {
            float rest = restitution.getValue();
            Vec3d particle = *particle_start;
            Vec3d new_pos = particle;
            new_pos[1] = max(planeHight+1,(new_pos[1] - t));

            *particle_pos = new_pos;
        }
    }


    // Debugging info
    /*if( t - prevT > .08 )
    printf("(!!) Dropped Frame %lf (!!)\n", t-prevT);*/
    prevT = t;
}


/** Render particles */
void ParticleSystem::drawParticles(float t)
{
    if(simulate)
    {
        Vec3d particle = *particle_pos;
        glPushMatrix();
        glTranslatef(particle[0],particle[1],particle[2]);
        drawSphere(1);
        glPopMatrix();
    }
}



/** Adds the current configuration of particles to
* your data structure for storing baked particles **/
void ParticleSystem::bakeParticles(float t) 
{
    // TODO (baking is extra credit)
}

/** Clears out your data structure of baked particles */
void ParticleSystem::clearBaked()
{
    // TODO (baking is extra credit)
}

void ParticleSystem::SpawnParticle(Vec3d point)
{
    //
    // Only spawn a new particle if we don't already have one.
    //
    if(particle_pos == NULL)
    {
        //printf("spawn particle (%f,%f,%f)\n", point[0], point[1], point[2]);
        particle_pos = new Vec3d(point);
        particle_start = new Vec3d(point);
    }
}

void ParticleSystem::setGroundPlane(double width, double depth, double hight)
{
    planeWidth = width;
    planeDepth = depth;
    planeHight = hight;
}




