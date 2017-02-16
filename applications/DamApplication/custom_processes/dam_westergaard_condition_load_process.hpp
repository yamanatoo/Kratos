//
//   Project Name:        KratosDamApplication    $
//   Last modified by:    $Author: Lorenzo Gracia $
//   Date:                $Date:        July 2016 $
//   Revision:            $Revision:          0.0 $
//

#if !defined(KRATOS_DAM_WESTERGAARD_CONDITION_LOAD_PROCESS )
#define  KRATOS_DAM_WESTERGAARD_CONDITION_LOAD_PROCESS

#include <cmath>

#include "includes/kratos_flags.h"
#include "includes/kratos_parameters.h"
#include "processes/process.h"

#include "dam_application_variables.h"

namespace Kratos
{

class DamWestergaardConditionLoadProcess : public Process
{
    
public:

    KRATOS_CLASS_POINTER_DEFINITION(DamWestergaardConditionLoadProcess);
    
    typedef Table<double,double> TableType;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /// Constructor
    DamWestergaardConditionLoadProcess(ModelPart& model_part,
                                Parameters rParameters
                                ) : Process(Flags()) , mr_model_part(model_part)
    {
        KRATOS_TRY
			 
        //only include validation with c++11 since raw_literals do not exist in c++03
        Parameters default_parameters( R"(
            {
                "model_part_name":"PLEASE_CHOOSE_MODEL_PART_NAME",
                "mesh_id": 0,
                "variable_name": "PLEASE_PRESCRIBE_VARIABLE_NAME",
                "is_fixed"                                              : false,
                "Modify"                                                : true,
                "Gravity_Direction"                                     : "Y",
                "Reservoir_Bottom_Coordinate_in_Gravity_Direction"      : 0.0,
                "Spe_weight"                                            : 0.0,
                "Water_level"                                           : 0.0,
                "Water_Table"                                           : 0, 
                "Aceleration"                                           : 0.0,
                "Aceleration_Table"                                     : 0  
            }  )" );
            
        
        // Some values need to be mandatorily prescribed since no meaningful default value exist. For this reason try accessing to them
        // So that an error is thrown if they don't exist
        rParameters["Reservoir_Bottom_Coordinate_in_Gravity_Direction"];
        rParameters["variable_name"];
        rParameters["model_part_name"];

        // Now validate agains defaults -- this also ensures no type mismatch
        rParameters.ValidateAndAssignDefaults(default_parameters);
        
        mmesh_id = rParameters["mesh_id"].GetInt();
        mvariable_name = rParameters["variable_name"].GetString();
        mis_fixed = rParameters["is_fixed"].GetBool();
        mgravity_direction = rParameters["Gravity_Direction"].GetString();
        mreference_coordinate = rParameters["Reservoir_Bottom_Coordinate_in_Gravity_Direction"].GetDouble();
        mspecific = rParameters["Spe_weight"].GetDouble();
        mwater_level = rParameters["Water_level"].GetDouble();
        macceleration = rParameters["Aceleration"].GetDouble();
        
        mtime_unit_converter = mr_model_part.GetProcessInfo()[TIME_UNIT_CONVERTER];
        mTableIdWater = rParameters["Water_Table"].GetInt();
        mTableIdAcceleration = rParameters["Aceleration_Table"].GetInt();
          
       if(mTableIdWater != 0)
            mpTableWater = model_part.pGetTable(mTableIdWater);
            
        if(mTableIdAcceleration != 0)
            mpTableAcceleration = model_part.pGetTable(mTableIdAcceleration);
        

        
        KRATOS_CATCH("");
    }

    ///------------------------------------------------------------------------------------
    
    /// Destructor
    virtual ~DamWestergaardConditionLoadProcess() {}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void Execute()
    {
    }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void ExecuteInitialize()
    {
        KRATOS_TRY;
        
        Variable<double> var = KratosComponents< Variable<double> >::Get(mvariable_name);
        const int nnodes = mr_model_part.GetMesh(mmesh_id).Nodes().size();
        int direction;
        double pressure;
        
        if( mgravity_direction == "X")
            direction = 1;
        else if( mgravity_direction == "Y")
            direction = 2;
        else if( mgravity_direction == "Z")
            direction = 3;
        
		double ref_coord = mreference_coordinate + mwater_level;
		double unit_acceleration = macceleration/9.81;
                   
        if(nnodes != 0)
        {
            ModelPart::NodesContainerType::iterator it_begin = mr_model_part.GetMesh(mmesh_id).NodesBegin();

            #pragma omp parallel for
            for(int i = 0; i<nnodes; i++)
            {
                ModelPart::NodesContainerType::iterator it = it_begin + i;

                if(mis_fixed)
                {
                    it->Fix(var);
                }

				double y_water =  ref_coord- (it->Coordinate(direction));
                
				if (y_water<0.0)
				{
					y_water=0.0;
				}
                
                // Hydrodynamics Westergaard effects just contribute when the acceleration goes in the upstream direction                
                if(unit_acceleration<0.0)
                {
                // The first part is related to hydrostatic pressure 
                     pressure = (mspecific*(y_water)) + 0.875*(-1.0*unit_acceleration)*mspecific*sqrt(y_water*mwater_level);
                }
                else
                {
                     pressure = (mspecific*(y_water));
                }
                
                if(pressure>0.0)
                {
                    it->FastGetSolutionStepValue(var) = pressure;
                }
                else
                {
                    it->FastGetSolutionStepValue(var)=0.0;
                }
            }            
        }
        
        KRATOS_CATCH("");
    }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void ExecuteInitializeSolutionStep()
    {
        
        KRATOS_TRY;
        
        Variable<double> var = KratosComponents< Variable<double> >::Get(mvariable_name);
        
        // Getting the values of table in case that it exist        
        if(mTableIdWater != 0)
        { 
            double time = mr_model_part.GetProcessInfo()[TIME];
            time = time/mtime_unit_converter;
            mwater_level = mpTableWater->GetValue(time);
        }
        
        if(mTableIdAcceleration != 0)
        { 
            double time = mr_model_part.GetProcessInfo()[TIME];
            time = time/mtime_unit_converter;
            macceleration = mpTableAcceleration->GetValue(time);
        }
        
        const int nnodes = mr_model_part.GetMesh(mmesh_id).Nodes().size();
        int direction;
        double pressure;
        
        if( mgravity_direction == "X")
            direction = 1;
        else if( mgravity_direction == "Y")
            direction = 2;
        else if( mgravity_direction == "Z")
            direction = 3;
        
		double ref_coord = mreference_coordinate + mwater_level;
		double unit_acceleration = macceleration/9.81;
                           
        if(nnodes != 0)
        {
            ModelPart::NodesContainerType::iterator it_begin = mr_model_part.GetMesh(mmesh_id).NodesBegin();

            #pragma omp parallel for
            for(int i = 0; i<nnodes; i++)
            {
                ModelPart::NodesContainerType::iterator it = it_begin + i;

                if(mis_fixed)
                {
                    it->Fix(var);
                }
                   
				double y_water =  ref_coord- (it->Coordinate(direction));
                
				if (y_water<0.0)
				{
					y_water=0.0;
				}
                
                // Hydrodynamics Westergaard effects just contribute when the acceleration goes in the upstream direction                
                if(unit_acceleration<0.0)
                {
                // The first part is related to hydrostatic pressure 
                    pressure = (mspecific*(y_water)) + 0.875*(-1.0*unit_acceleration)*mspecific*sqrt(y_water*mwater_level);
                }
                else
                {
                    pressure = (mspecific*(y_water));
                }
                
                if(pressure>0.0)
                {
                    it->FastGetSolutionStepValue(var) = pressure;
                }
                else
                {
                    it->FastGetSolutionStepValue(var)=0.0;
                }
            }            
        }
        
        KRATOS_CATCH("");
    }
    
    /// Turn back information as a string.
    std::string Info() const
    {
        return "DamWestergaardConditionLoadProcess";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << "DamWestergaardConditionLoadProcess";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const
    {
    }

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

protected:

    /// Member Variables

    ModelPart& mr_model_part;
    std::size_t mmesh_id;
    std::string mvariable_name;
    std::string mgravity_direction;
    bool mis_fixed;
    double mreference_coordinate;
    double mspecific;
    double mwater_level;
    double macceleration;
    double mtime_unit_converter;
    TableType::Pointer mpTableWater;
    TableType::Pointer mpTableAcceleration;
    int mTableIdWater;
	int mTableIdAcceleration;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

private:

    /// Assignment operator.
    DamWestergaardConditionLoadProcess& operator=(DamWestergaardConditionLoadProcess const& rOther);

};//Class


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  DamWestergaardConditionLoadProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const DamWestergaardConditionLoadProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

} /* namespace Kratos.*/

#endif /* KRATOS_DAM_WESTERGAARD_CONDITION_LOAD_PROCESS defined */
