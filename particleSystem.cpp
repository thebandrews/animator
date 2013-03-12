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
    restitution("Restitution", 0.0f, 2.0f, 0.9f, 0.1f)
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

    // These values are used by the UI
    simulate = false;
    dirty = true;

}

/** Reset the simulation */
void ParticleSystem::resetSimulation(float t)
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

    // These values are used by the UI
    //simulate = false;
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
            std::vector<Particle*>::iterator iter;

            for(iter = particles_.begin(); iter != particles_.end(); iter++)
            {
                EulerStep(t - prevT);
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
            drawSphere((*iter)->radius);
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
    // Only spawn new particle if we don't already have some.
    //
    if(particles_.size() == 0)
    {
        Particle* p1 = new Particle;
        p1->m = 0.8;  // Set some arbitrary mass
        p1->radius = 1.0;
        p1->x = new Vec3d(point);
        p1->start = new Vec3d(point);
        p1->v = new Vec3d(0,20,35); // Test some starting velocities(0,20,15) is a good start...
        p1->f = new Vec3d(0,0,0);
        particles_.push_back(p1);
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
    CollisionDetection(temp2);
    ParticleSetState(temp2); /* update state */

    delete[] temp1;
    delete[] temp2;
}


void ParticleSystem::CollisionDetection(double *src)
{
    const float SURFACE_EPSILON = 0.5;
    float rest = restitution.getValue();
    Vec3d N = planeNormal;

    int i;
    for(i=0; i < particles_.size(); i++)
    {
        Vec3d P(*(src), *(src+1), *(src+2));
        P[1] -= particles_[i]->radius;

        //
        // Check to see if the particle is still above the plane
        //
        double plane_edge_x = planePos[0]/2;
        double plane_edge_z = planePos[1]/2;

        if(!((P[0] > plane_edge_x) || (P[0] < -plane_edge_x) ||
           (P[2] > plane_edge_z) || (P[2] < -plane_edge_z)))
        {
            Vec3d X(P);
            X[1] = planePos[2];

            double XdotP = (X-P)*N;

            //
            // Check to see if we have a collision
            //
            if(XdotP <= SURFACE_EPSILON)
            {
                Vec3d V(*(src+3), *(src+4), *(src+5));

                double NdotV = (N*V);
                Vec3d v_n = NdotV*N;

                Vec3d v_t = V-v_n;

                Vec3d v_p = v_t-((rest)*v_n);

                if(v_p*N <= 0)
                {
                    *(src+3) = v_p[0];
                    *(src+4) = v_p[1];
                    *(src+5) = v_p[2];
                    *(src+1) = planePos[2] + (particles_[i]->radius);
                }
                else
                {
                    *(src+4) = 0.0;
                    *(src+1) = planePos[2] + (particles_[i]->radius);
                }
            }
        }
    }
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
    double k_drag = -0.5; //Some arbitrary drag coefficient.
    int i;
    for(i=0; i < particles_.size(); i++)
    {
        double f_x = 0.0;
        double f_y = 0.0; //-150*(particles_[i]->m); 
        double f_z = 0.0;

        //
        // Compute Force #1 Gravity
        //
        f_y = -150*(particles_[i]->m); //Gravitational constant in the -y direction

        //
        // Compute Force #2 drag
        //
        f_x = (*particles_[i]->v)[0]*k_drag;
        f_y = f_y + (*particles_[i]->v)[1]*k_drag;
        f_z =  (*particles_[i]->v)[2]*k_drag;

        //
        // Store the force values in the particle struct
        //
        (*particles_[i]->f)[0] = f_x;
        (*particles_[i]->f)[1] = f_y;
        (*particles_[i]->f)[2] = f_z;
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


void ParticleSystem::setGroundPlane(Vec3d position, Vec3d normal)
{
    planePos = position;
    planeNormal = normal;
}

