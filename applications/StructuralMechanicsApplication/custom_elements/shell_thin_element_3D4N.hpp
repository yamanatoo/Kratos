// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Peter Wilson
//       Contact:    A.Winterstein@tum.de
//

#if !defined(SHELL_THIN_ELEMENT_3D4N_H_INCLUDED )
#define  SHELL_THIN_ELEMENT_3D4N_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/element.h"
#include "custom_utilities/shell_cross_section.hpp"
#include "utilities/quaternion.h"
#include "custom_utilities/shellq4_local_coordinate_system.hpp"
//#include "custom_utilities/shellq4_corotational_coordinate_transformation.hpp"

namespace Kratos
{
	///@name Kratos Globals
	///@{
	///@}

	///@name Type Definitions
	///@{
	///@}

	class ShellQ4_CoordinateTransformation;

	class ShellQ4_LocalCoordinateSystem;

	///@name  Enum's
	///@{
	///@}

	///@name  Functions
	///@{
	///@}

	///@name Kratos Classes
	///@{
	/** \brief ShellThinElement3D4N
	*
	* This element represents a 4-node Shell element
	* based on the Assumed Natural DEviatoric Strain (ANDES) by Felippa.
	* This element is formulated for small strains,
	* but can be used in Geometrically nonlinear problems
	* involving large displacements and rotations
	* using a Corotational Coordinate Transformation.
	* Material nonlinearity is handled by means of the cross section object.
	*/
	class ShellThinElement3D4N : public Element
	{
	public:

		///@name Type Definitions
		///@{
		KRATOS_CLASS_POINTER_DEFINITION(ShellThinElement3D4N);

		typedef std::vector< ShellCrossSection::Pointer > CrossSectionContainerType;

		typedef ShellQ4_CoordinateTransformation CoordinateTransformationBaseType;

		typedef boost::shared_ptr<CoordinateTransformationBaseType> CoordinateTransformationBasePointerType;

		typedef array_1d<double, 3> Vector3Type;

		typedef Quaternion<double> QuaternionType;

		///@}

		///@name Classes
		///@{
		/** \brief JacobianOperator
		*
		* This class is a utility to compute at a given integration point,
		* the Jacobian, its inverse, its determinant
		* and the derivatives of the shape functions in the local
		* cartesian coordinate system.
		*/

		class JacobianOperator
		{
		public:

			JacobianOperator();

			void Calculate(const ShellQ4_LocalCoordinateSystem & CS, const Matrix & dN);

			inline const Matrix & Jacobian()const
			{
				return mJac;
			}

			inline const Matrix & Inverse()const
			{
				return mInv;
			}

			inline const Matrix & XYDerivatives()const
			{
				return mXYDeriv;
			}

			inline const double Determinant()const
			{
				return mDet;
			}

		private:

			Matrix mJac;     //!< Jacobian matrix
			Matrix mInv;    //!< Inverse of the Jacobian matrix
			Matrix mXYDeriv; //*!< Shape function derivatives in cartesian coordinates
			double mDet;     //*!< Determinant of the Jacobian matrix
		};

		// TODO: Add Calculation Data

		///@}

		///@name Life Cycle
		///@{
		ShellThinElement3D4N(IndexType NewId,
			GeometryType::Pointer pGeometry,
			bool NLGeom = false);

		ShellThinElement3D4N(IndexType NewId,
			GeometryType::Pointer pGeometry,
			PropertiesType::Pointer pProperties,
			bool NLGeom = false);

		ShellThinElement3D4N(IndexType NewId,
			GeometryType::Pointer pGeometry,
			PropertiesType::Pointer pProperties,
			CoordinateTransformationBasePointerType pCoordinateTransformation);

		virtual ~ShellThinElement3D4N();

		///@}

		///@name Operations
		///@{
		// Basic

		Element::Pointer Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const;

		IntegrationMethod GetIntegrationMethod() const;

		void Initialize();

		//void ResetConstitutiveLaw();

		void EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& rCurrentProcessInfo);

		void GetDofList(DofsVectorType& ElementalDofList, ProcessInfo& CurrentProcessInfo);

		int Check(const ProcessInfo& rCurrentProcessInfo);

		void CleanMemory();

		void GetValuesVector(Vector& values, int Step = 0);
		// needed for dyn

		void GetFirstDerivativesVector(Vector& values, int Step = 0);
		//needed for dyn

		void GetSecondDerivativesVector(Vector& values, int Step = 0);
		//needed for dyn

		void InitializeNonLinearIteration(ProcessInfo& CurrentProcessInfo);
		//needed for corotational

		void FinalizeNonLinearIteration(ProcessInfo& CurrentProcessInfo);
		//needed for corotational

		void InitializeSolutionStep(ProcessInfo& CurrentProcessInfo);
		//needed for corotational

		void FinalizeSolutionStep(ProcessInfo& CurrentProcessInfo);
		//needed for corotational

		void CalculateMassMatrix(MatrixType& rMassMatrix,
			ProcessInfo& rCurrentProcessInfo);
		// needed for dyn

		void CalculateDampingMatrix(MatrixType& rDampingMatrix,
			ProcessInfo& rCurrentProcessInfo);
		// needed for dyn

		void CalculateLocalSystem(MatrixType& rLeftHandSideMatrix,
			VectorType& rRightHandSideVector,
			ProcessInfo& rCurrentProcessInfo);

		void CalculateRightHandSide(VectorType& rRightHandSideVector,
			ProcessInfo& rCurrentProcessInfo);

		void CalculateGeometricStiffnessMatrix(MatrixType& rGeometricStiffnessMatrix,
			ProcessInfo& rCurrentProcessInfo);
		// needed for dyn

		// Results calculation on integration points
		void GetValueOnIntegrationPoints(const Variable<double>& rVariable,
			std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void GetValueOnIntegrationPoints(const Variable<Vector>& rVariable,
			std::vector<Vector>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void GetValueOnIntegrationPoints(const Variable<Matrix>& rVariable,
			std::vector<Matrix>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void GetValueOnIntegrationPoints(const Variable<array_1d<double,
			3> >& rVariable, std::vector<array_1d<double, 3> >& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		void GetValueOnIntegrationPoints(const Variable<array_1d<double,
			6> >& rVariable, std::vector<array_1d<double, 6> >& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		// More results calculation on integration points to interface with python
		void CalculateOnIntegrationPoints(const Variable<double>& rVariable,
			std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void CalculateOnIntegrationPoints(const Variable<Vector>& rVariable,
			std::vector<Vector>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void CalculateOnIntegrationPoints(const Variable<Matrix>& rVariable,
			std::vector<Matrix>& rValues, const ProcessInfo& rCurrentProcessInfo);

		void CalculateOnIntegrationPoints(const Variable<array_1d<double,
			3> >& rVariable, std::vector<array_1d<double, 3> >& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		void CalculateOnIntegrationPoints(const Variable<array_1d<double,
			6> >& rVariable, std::vector<array_1d<double, 6> >& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		///@}

		std::vector<Properties> mPly_Properties;

		///@name Public specialized Access - Temporary
		///@{
		///@}

	protected:

		///@name Protected Lyfe Cycle
		///@{
		/**
		* Protected empty constructor
		*/
		ShellThinElement3D4N() : Element()
		{
		}

		///@}

	private:

		///@name Private Classes
		///@{
		class CalculationData
		{
		public:

			// ---------------------------------------
			// calculation-constant data
			// ----------------------------------------
			// these data are allocated and constructed
			// at the beginning of the calculation

			ShellQ4_LocalCoordinateSystem LCS;  /*!< current coordinate system */
			ShellQ4_LocalCoordinateSystem LCS0; /*!< reference coordinate system */

			// Unit vectors (in cartesian coords)
			Vector s_xi = ZeroVector(3);	/*!< xi unit vector in cartesian coords */
			Vector s_eta = ZeroVector(3);	/*!< xi unit vector in cartesian coords */
			
			// Geometry data
			array_1d<Vector, 4> r_cartesian;	/*!< array of cartesian point positions */
			array_1d<double, 4> dA;	/*!< array of integration areas (incl. weight) */

			// Displacements
			VectorType globalDisplacements = ZeroVector(24); /*!< global displacement vector */
			VectorType localDisplacements = ZeroVector(24);  /*!< local displacement vector */

			// Element flags
			bool CalculateRHS; /*!< flag for the calculation of the right-hand-side vector */
			bool CalculateLHS; /*!< flag for the calculation of the left-hand-side vector */

			// ---------------------------------------
			// Testing flags
			// ---------------------------------------
			// These should both be FALSE unless you are testing, or 
			// investigating the effects of element enhancements!

			const bool basicQuad = false;	/*!< flag for using basic membrane 
											formulation - should be FALSE unless
											you are testing */

			// ---------------------------------------
			// calculation-variable data
			// ---------------------------------------
			// these data are updated during the
			// calculations

			array_1d<double, 4> N;	/*!< SF values at parametric point */
			size_t gpIndex;	/*!< Index of current Gauss Point (zero based) */

			// ---------------------------------------
			// calculation-variable data
			// ---------------------------------------
			// these data are updated during the
			// calculations, but they are allocated
			// only once(the first time they are used)
			// to avoid useless re-allocations

			// ANDES membrane data
			const double alpha = 1.5;
			MatrixType L_mem = ZeroMatrix(3, 12); /*!< basic membrane lumping matrix */
			MatrixType H_mem_mod = ZeroMatrix(7, 12);	/*!< higher order membrane filter matrix */
			MatrixType Z = ZeroMatrix(12, 12);	/*!< transformation from Haugen to Kratos DOFs */
			MatrixType B_h_1 = ZeroMatrix(3, 7);	/*!< higher order membrane B1 matrix */
			MatrixType B_h_2 = ZeroMatrix(3, 7);	/*!< higher order membrane B2 matrix */
			MatrixType B_h_3 = ZeroMatrix(3, 7);	/*!< higher order membrane B3 matrix */
			MatrixType B_h_4 = ZeroMatrix(3, 7);	/*!< higher order membrane B4 matrix */
			MatrixType B_h_bar = ZeroMatrix(3, 7);	/*!< higher order membrane B_bar matrix */
			MatrixType T_13 = ZeroMatrix(3, 3);
			MatrixType T_24 = ZeroMatrix(3, 3);
			
			// DKQ bending data
			array_1d<double, 4> DKQ_a;
			array_1d<double, 4> DKQ_b;
			array_1d<double, 4> DKQ_c;
			array_1d<double, 4> DKQ_d;
			array_1d<double, 4> DKQ_e;
			MatrixType DKQ_indices = ZeroMatrix(4, 2);
			array_1d<Matrix, 4> DKQ_invJac;
			array_1d<Matrix, 4> DKQ_jac;
			array_1d<double, 4> DKQ_jac_det;


			// General total element data
			MatrixType B = ZeroMatrix(6, 24);   /*!< total strain-displacement matrix at the current integration point */
			MatrixType D = ZeroMatrix(6, 6);    /*!< section constitutive matrix at the current integration point */
			MatrixType BTD = ZeroMatrix(24, 6);  /*!< auxiliary matrix to store the product B'*D */

			VectorType generalizedStrains = ZeroVector(6);  /*!< generalized strain vector at the current integration point */
			VectorType generalizedStresses = ZeroVector(6); /*!< generalized stress vector at the current integration point */
			std::vector<VectorType> rlaminateStrains;	/*!< laminate strain vector at all surfaces at the current integration point */
			std::vector<VectorType> rlaminateStresses;	/*!< laminate stress vector at all surfaces at the current integration point */

			JacobianOperator jacOp;
			ShellCrossSection::Parameters SectionParameters; /*!< parameters for cross section calculations */

		public:

			const ProcessInfo& CurrentProcessInfo;

		public:

			CalculationData(const ShellQ4_LocalCoordinateSystem& localcoordsys,
				const ShellQ4_LocalCoordinateSystem& refcoordsys,
				const ProcessInfo& rCurrentProcessInfo);
		};

		///@}

		///@name Private Operations
		///@{
		void CalculateStressesFromForceResultants
			(VectorType& rstresses,
				const double& rthickness);

		void CalculateLaminaStrains(CalculationData& data);

		void CalculateLaminaStresses(CalculationData& data);

		double CalculateTsaiWuPlaneStress(const CalculationData& data, const Matrix& rLamina_Strengths, const unsigned int& rCurrent_Ply);

		void CalculateVonMisesStress(const CalculationData& data, const Variable<double>& rVariable, double& rVon_Mises_Result);

		void CalculateShellElementEnergy(const CalculationData& data, const Variable<double>& rVariable, double& rEnergy_Result);

		void CheckGeneralizedStressOrStrainOutput(const Variable<Matrix>& rVariable, int& iJob, bool& bGlobal);

		void DecimalCorrection(Vector& a);

		void SetupOrientationAngles();

		void InitializeCalculationData(CalculationData& data);

		void CalculateBMatrix(CalculationData& data);

		void CalculateSectionResponse(CalculationData& data);

		void CalculateGaussPointContribution(CalculationData& data, 
			MatrixType& LHS, VectorType& RHS);

		void AddBodyForces(CalculationData& data, 
			VectorType& rRightHandSideVector); //not required for dyn

		void CalculateAll(MatrixType& rLeftHandSideMatrix,
			VectorType& rRightHandSideVector,
			ProcessInfo& rCurrentProcessInfo,
			const bool LHSrequired,
			const bool RHSrequired);

		bool TryGetValueOnIntegrationPoints_MaterialOrientation(const Variable<array_1d<double, 3> >& rVariable,
			std::vector<array_1d<double, 3> >& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		bool TryGetValueOnIntegrationPoints_GeneralizedStrainsOrStresses(const Variable<Matrix>& rVariable,
			std::vector<Matrix>& rValues,
			const ProcessInfo& rCurrentProcessInfo);

		void printMatrix(Matrix& matrixIn, std::string stringIn);

		void printVector(Vector& matrixIn, std::string stringIn);

		void printMatrix(const Matrix& matrixIn, std::string stringIn);

		void printDouble(std::string stringIn, double doubleIn);

		///@}

		///@name Static Member Variables
		///@{
		///@}

		///@name Member Variables
		///@{
		CoordinateTransformationBasePointerType mpCoordinateTransformation; /*!< The Coordinate Transformation */

		CrossSectionContainerType mSections; /*!< Container for cross section associated to each integration point */

		IntegrationMethod mThisIntegrationMethod; /*!< Currently selected integration method */

												  ///@}

												  ///@name Serialization
												  ///@{
		friend class Serializer;

		virtual void save(Serializer& rSerializer) const;

		virtual void load(Serializer& rSerializer);

		///@}

		///@name Private  Access
		///@{
		///@}

		///@name Private Inquiry
		///@{
		///@}

		///@name Un accessible methods
		///@{
		///@}
	};
}
#endif // SHELL_THIN_ELEMENT_3D4N_H_INCLUDED