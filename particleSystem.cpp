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
    particles_.clear();

    // Leave these here; the UI needs them to work correctly.
    dirty = false;
    simulate = false;
}




/*************
* Destructor
*************/

ParticleSystem::~ParticleSystem() 
{    
    std::vector<Particle*>::iterator iter;

    //
    // Clean up particles
    //
    for(iter = particles_.begin(); iter != particles_.end(); iter++)
    {
        delete (*iter)->x;
        delete (*iter)->start;
        delete (*iter)->v;
        delete (*iter)->f;
    }

    particles_.clear();

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
    std::vector<Particle*>::iterator iter;

    //
    // Clean up particles
    //
    for(iter = particles_.begin(); iter != particles_.end(); iter++)
    {
        delete (*iter)->x;
        delete (*iter)->start;
        delete (*iter)->v;
        delete (*iter)->f;
    }

    particles_.clear();

    // These values are used by the UI
    simulate = false;
    dirty = true;

}

/** Reset the simulation */
void ParticleSystem::resetSimulation(float t)
{
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

            std::vector<Particle*>::iterator iter;

            for(iter = particles_.begin(); iter != particles_.end(); iter++)
            {
                EulerStep(t - prevT);
                /*Vec3d particle = (*(*iter)->x);
                Vec3d new_pos = particle;
                new_pos[1] = max(planeHight+1,(new_pos[1] - t));

                (*(*iter)->x)[0] = new_pos[0];
                (*(*iter)->x)[1] = new_pos[1];
                (*(*iter)->x)[2] = new_pos[2];*/
            }
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
        std::vector<Particle*>::iterator iter;

        for(iter = particles_.begin(); iter != particles_.end(); iter++)
        {
            Vec3d particle = (*(*iter)->x);
            glPushMatrix();
            glTranslatef(particle[0],particle[1],particle[2]);
            drawSphere(1);
            glPopMatrix();
        }
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
    if(particles_.size() == 0)
    {
        printf("spawn particle (%f,%f,%f)\n", point[0], point[1], point[2]);
        Particle* p = new Particle;
        p->m = 1;
        p->x = new Vec3d(point);
        p->start = new Vec3d(point);
        p->v = new Vec3d(0,0,0);
        p->f = new Vec3d(0,0,0);
        particles_.push_back(p);

        /*particle_pos = new Vec3d(point);
        particle_start = new Vec3d(point);*/
    }
}

/* length of state derivative, and force vectors */
int ParticleSystem::ParticleDims()
{
    return(6 * particles_.size());
}


/* gather state from the particles into dst */
void ParticleSystem::ParticleGetState(double *dst)
{
    int i;
    for(i=0; i < particles_.size(); i++){
        *(dst++) = (*particles_[i]->x)[0];
        *(dst++) = (*particles_[i]->x)[1];
        *(dst++) = (*particles_[i]->x)[2];
        *(dst++) = (*particles_[i]->v)[0];
        *(dst++) = (*particles_[i]->v)[1];
        *(dst++) = (*particles_[i]->v)[2];
    }
}


/* scatter state from src into the particles */
void ParticleSystem::ParticleSetState(double *src)
{
    int i;
    for(i=0; i < particles_.size(); i++){
        (*particles_[i]->x)[0] = *(src++);
        (*particles_[i]->x)[1] = *(src++);
        (*particles_[i]->x)[2] = *(src++);
        (*particles_[i]->v)[0] = *(src++);
        (*particles_[i]->v)[1] = *(src++);
        (*particles_[i]->v)[2] = *(src++);
    }
}


/* calculate derivative, place in dst */
void ParticleSystem::ParticleDerivative(double *dst)
{
    int i;
    ClearForces(); /* zero the force accumulators */
    ComputeForces(); /* magic force function */
    for(i=0; i < particles_.size(); i++){
        *(dst++) = (*particles_[i]->v)[0]; /* xdot = v */
        *(dst++) = (*particles_[i]->v)[1];
        *(dst++) = (*particles_[i]->v)[2];
        *(dst++) = (*particles_[i]->f)[0]/(particles_[i]->m); /* vdot = f/m */
        *(dst++) = (*particles_[i]->f)[1]/(particles_[i]->m);
        *(dst++) = (*particles_[i]->f)[2]/(particles_[i]->m);
    }
}


void ParticleSystem::EulerStep(double DeltaT)
{
    int buffer_size = ParticleDims();
    double* temp1 = new double[buffer_size];
    double* temp2 = new double[buffer_size];

    ParticleDerivative(temp1); /* get deriv */
    ScaleVector(temp1,buffer_size, DeltaT); /* scale it */
    ParticleGetState(temp2); /* get state */
    AddVectors(temp1,temp2,temp2,buffer_size); /* add -> temp2 */
    ParticleSetState(temp2); /* update state */
    //p->t += DeltaT; /* update time */

    delete[] temp1;
    delete[] temp2;
}


void ParticleSystem::ClearForces()
{
    int i;
    for(i=0; i < particles_.size(); i++)
    {
        (*particles_[i]->f)[0] = 0.0;
        (*particles_[i]->f)[1] = 0.0;
        (*particles_[i]->f)[2] = 0.0;
    }
}


void ParticleSystem::ComputeForces()
{
    int i;
    for(i=0; i < particles_.size(); i++)
    {
        (*particles_[i]->f)[0] = 0.0;
        (*particles_[i]->f)[1] = -150;  //Gravitational constant in the -y direction
        (*particles_[i]->f)[2] = 0.0;
    }
}


void ParticleSystem::ScaleVector(double *src, int size, double scale)
{
    int i;
    for(i=0; i < size; i++)
    {
        (*src) = *(src++)*scale;
    }
}

void ParticleSystem::AddVectors(double *a, double *b, double *dst, int size)
{
    int i;
    for(i=0; i < size; i++)
    {
        *(dst++) = *(a++)+*(b++);
    }
}


void ParticleSystem::setGroundPlane(double width, double depth, double hight)
{
    planeWidth = width;
    planeDepth = depth;
    planeHight = hight;
}

