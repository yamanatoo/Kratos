//Authors: M.A. Celigueta and S. Latorre (CIMNE)
//   Date: July 2015

#if !defined(DEM_D_HERTZ_VISCOUS_COULOMB_CL_H_INCLUDED)
#define  DEM_D_HERTZ_VISCOUS_COULOMB_CL_H_INCLUDED

#include <string>
#include <iostream>

#include "DEM_application.h"
#include "DEM_discontinuum_constitutive_law.h"
#include "../custom_elements/spheric_particle.h"

namespace Kratos {

    class DEM_D_Hertz_viscous_Coulomb : public DEMDiscontinuumConstitutiveLaw {
    public:

        KRATOS_CLASS_POINTER_DEFINITION(DEM_D_Hertz_viscous_Coulomb);

        DEM_D_Hertz_viscous_Coulomb() {
        }
      
        void Initialize(const ProcessInfo& rCurrentProcessInfo);         

        void SetConstitutiveLawInProperties(Properties::Pointer pProp) const;

        ~DEM_D_Hertz_viscous_Coulomb() {
        }

        DEMDiscontinuumConstitutiveLaw::Pointer Clone() const;      
        
        void InitializeContact(SphericParticle* const element1, SphericParticle* const element2, const double indentation);  
        
        void InitializeContactWithFEM(SphericParticle* const element, DEMWall* const wall, const double indentation, const double ini_delta = 0.0);
        void CalculateForces(const double OldLocalContactForce[3],
                            double LocalElasticContactForce[3],
                            double LocalDeltDisp[3],
                            double LocalRelVel[3],            
                            double indentation,
                            double previous_indentation,
                            double ViscoDampingLocalContactForce[3],
                            double& cohesive_force,
                            SphericParticle* element1,
                            SphericParticle* element2); 
        
        void CalculateForcesWithFEM(const double OldLocalContactForce[3], double LocalElasticContactForce[3],
                                    double LocalDeltDisp[3], double LocalRelVel[3], double indentation,
                                    double previous_indentation, double ViscoDampingLocalContactForce[3], double& cohesive_force,
                                    SphericParticle* const element, DEMWall* const wall);
        
        double CalculateNormalForce(const double indentation);
        
        double CalculateCohesiveNormalForce(SphericParticle* const element1,
                                            SphericParticle* const element2);
        double CalculateCohesiveNormalForceWithFEM(SphericParticle* const element,
                                            DEMWall* const wall);
        
        void CalculateTangentialForce(const double normal_contact_force,
                                    const double OldLocalContactForce[3],
                                    double LocalElasticContactForce[3],
                                    //const double ViscoDampingLocalContactForce[3],
                                    double ViscoDampingLocalContactForce[3],
                                    const double LocalDeltDisp[3],            
                                    bool& sliding,
                                    SphericParticle* const element1,
                                    SphericParticle* const element2,
                                    double indentation,
                                    double previous_indentation);
        
        void CalculateTangentialForceWithFEM(const double normal_contact_force,
                                             const double OldLocalContactForce[3],
                                      double LocalElasticContactForce[3],
                                      double ViscoDampingLocalContactForce[3],
                                      const double LocalDeltDisp[3],            
                                      bool& sliding,
                                      SphericParticle* const element,
                                      DEMWall* const wall,
                                      double indentation,
                                      double previous_indentation);
        
        void CalculateViscoDampingForce(double LocalRelVel[3],
                                        double ViscoDampingLocalContactForce[3],
                                        bool sliding,
                                        SphericParticle* const element1,
                                        SphericParticle* const element2,
                                        double indentation);            
        
        void CalculateViscoDampingForceWithFEM(double LocalRelVel[3],
                                        double ViscoDampingLocalContactForce[3],
                                        bool sliding,
                                        SphericParticle* const element,
                                        DEMWall* const wall,
                                        double indentation);
                            
    private:

        friend class Serializer;
        
        virtual void save(Serializer& rSerializer) const {
            KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, DEMDiscontinuumConstitutiveLaw)
                    //rSerializer.save("MyMemberName",myMember);
        }

        virtual void load(Serializer& rSerializer) {
            KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, DEMDiscontinuumConstitutiveLaw)
                    //rSerializer.load("MyMemberName",myMember);
        }
    };

} /* namespace Kratos.*/
#endif /* DEM_D_HERTZ_VISCOUS_COULOMB_CL_H_INCLUDED  defined */
