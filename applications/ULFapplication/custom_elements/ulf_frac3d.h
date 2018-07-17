//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pavel Ryzhakov

#if !defined(KRATOS_ULF_FRAC3D_H_INCLUDED )
#define  KRATOS_ULF_FRAC3D_H_INCLUDED



// System includes


// External includes
#include "boost/smart_ptr.hpp"


// Project includes
#include "includes/define.h"
#include "includes/element.h"
#include "includes/ublas_interface.h"
#include "includes/variables.h"


namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/
class UlfFrac3D
    : public Element
{
public:
    ///@name Type Definitions
    ///@{

    /// Counted pointer of UlfFrac3D
    KRATOS_CLASS_POINTER_DEFINITION(UlfFrac3D);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    UlfFrac3D(IndexType NewId, GeometryType::Pointer pGeometry);
    UlfFrac3D(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties);

    /// Destructor.
    ~UlfFrac3D() override;


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    Element::Pointer Create(IndexType NewId, NodesArrayType const& ThisNodes,  PropertiesType::Pointer pProperties) const;

    void CalculateLocalSystem(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);

    void CalculateRightHandSide(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);
    //virtual void CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo);

    void EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& rCurrentProcessInfo);

    void GetDofList(DofsVectorType& ElementalDofList,ProcessInfo& CurrentProcessInfo);

    void InitializeSolutionStep(ProcessInfo& CurrentProcessInfo);

    void Calculate(const Variable<double >& rVariable, double& Output, const ProcessInfo& rCurrentProcessInfo);
    void CalculateMassMatrix(MatrixType& rMassMatrix, ProcessInfo& rCurrentProcessInfo);
    void CalculateDampingMatrix(MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo);
    void GetValuesVector(Vector& values, int Step = 0);
    void GetFirstDerivativesVector(Vector& values, int Step = 0);
    void GetSecondDerivativesVector(Vector& values, int Step = 0);


    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "UlfFrac3D #" ;
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info() << Id();
    }

    /// Print object's data.
//      virtual void PrintData(std::ostream& rOStream) const;


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{
    //static boost::numeric::ublas::bounded_matrix<double,3,3> msMassFactors;
    //static boost::numeric::ublas::bounded_matrix<double,3,2> msDN_Dx;
    //static array_1d<double,3> msN; //dimension = number of nodes
    //static Matrix msDN_DX;
    //static Matrix msMassFactors;
    //static array_1d<double,2> ms_vel_gauss; //dimesion coincides with space dimension
    //static array_1d<double,3> ms_temp_vec_np; //dimension = number of nodes
    //static array_1d<double,3> ms_u_DN;


    //static boost::numeric::ublas::bounded_matrix<double,3,6> msB;
    //static boost::numeric::ublas::bounded_matrix<double,3,3> ms_constitutive_matrix;
    //static boost::numeric::ublas::bounded_matrix<double,3,6> ms_temp;
    //static array_1d<double,6> ms_temp_vec;


    ///@}
    ///@name Member Variables
    ///@{
    double mA0; //stores area at the beginning of time-step



    ///@}
    ///@name Private Operators
    ///@{
    void CalculateLumpedMass();
    void PressureStep(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);
    void VelocityStep(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);
    //inline void CalculateGeometryData(Matrix& msDN_DX, Vector& N, double& Area)
    //inline void CalculateGeometryData(boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, array_1d<double,4>& N, double& Area);

    ///@}
    ///@name Serialization
    ///@{
    friend class Serializer;


    // default constructor for serialization
    UlfFrac3D():Element() {}

    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    //UlfFrac3D& operator=(const UlfFrac3D& rOther);

    /// Copy constructor.
    //UlfFrac3D(const UlfFrac3D& rOther);


    ///@}

}; // Class UlfFrac3D

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
/*  inline std::istream& operator >> (std::istream& rIStream,
				    UlfFrac3D& rThis);
*/
/// output stream function
/*  inline std::ostream& operator << (std::ostream& rOStream,
				    const UlfFrac3D& rThis)
    {
      rThis.PrintInfo(rOStream);
      rOStream << std::endl;
      rThis.PrintData(rOStream);

      return rOStream;
    }*/
///@}

}  // namespace Kratos.

#endif // KRATOS_ULF_FRAC3D_H_INCLUDED   defined 


