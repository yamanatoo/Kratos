//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Ruben Zorrilla
//

#include "navier_stokes_wall_condition.h"

namespace Kratos
{

///@name Specialized implementation of VMS for functions that depend on TDim
///@{

/**
 * @see NavierStokesWallCondition::EquationIdVector
 */
template <>
void NavierStokesWallCondition<2,2>::EquationIdVector(EquationIdVectorType& rResult,
                                                      ProcessInfo& rCurrentProcessInfo)
{
    const unsigned int NumNodes = 2;
    const unsigned int LocalSize = 6;
    unsigned int LocalIndex = 0;

    if (rResult.size() != LocalSize)
        rResult.resize(LocalSize, false);

    for (unsigned int iNode = 0; iNode < NumNodes; ++iNode)
    {
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(VELOCITY_X).EquationId();
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(VELOCITY_Y).EquationId();
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(PRESSURE).EquationId();
    }
}

/**
 * @see NavierStokesWallCondition::EquationIdVector
 */
template <>
void NavierStokesWallCondition<3,3>::EquationIdVector(EquationIdVectorType& rResult,
                                                      ProcessInfo& rCurrentProcessInfo)
{
    const SizeType NumNodes = 3;
    const SizeType LocalSize = 12;
    unsigned int LocalIndex = 0;

    if (rResult.size() != LocalSize)
        rResult.resize(LocalSize, false);

    for (unsigned int iNode = 0; iNode < NumNodes; ++iNode)
    {
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(VELOCITY_X).EquationId();
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(VELOCITY_Y).EquationId();
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(VELOCITY_Z).EquationId();
        rResult[LocalIndex++] = this->GetGeometry()[iNode].GetDof(PRESSURE).EquationId();
    }
}

/**
 * @see NavierStokesWallCondition::GetDofList
 */
template <>
void NavierStokesWallCondition<2,2>::GetDofList(DofsVectorType& rElementalDofList,
                                                ProcessInfo& rCurrentProcessInfo)
{
    const SizeType NumNodes = 2;
    const SizeType LocalSize = 6;

    if (rElementalDofList.size() != LocalSize)
        rElementalDofList.resize(LocalSize);

    unsigned int LocalIndex = 0;

    for (unsigned int iNode = 0; iNode < NumNodes; ++iNode)
    {
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_X);
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Y);
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(PRESSURE);
    }
}

/**
 * @see NavierStokesWallCondition::Initialize
 * 
 * @tparam TDim 
 * @tparam TNumNodes 
 */

template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::Initialize()
{
	KRATOS_TRY;
	const array_1d<double,3>& rNormal = this->GetValue(NORMAL);
     // std::cout << rNormal << std::endl;
	if (norm_2(rNormal) == 0.0){
	    std::cout << "error on condition -> " << this->Id() << std::endl;
	    KRATOS_THROW_ERROR(std::logic_error, "NORMAL must be calculated before using this condition","");
	}
	if (mInitializeWasPerformed){
        KRATOS_ERROR_IF(mpParentElement == NULL) << "Parent as NULL pointer (Check 1)";
	    return;
	}

	mInitializeWasPerformed = true;
	// double EdgeLength;
	array_1d<double,3> Edge;
	GeometryType& rGeom = this->GetGeometry();
	WeakPointerVector<Element> ElementCandidates;
    
    // std::cout << "before finding candidates" << std::flush;
	for (SizeType i = 0; i < TNumNodes; i++){
		WeakPointerVector<Element>& rNodeElementCandidates = rGeom[i].GetValue(NEIGHBOUR_ELEMENTS);

         // Check that the condition has candidate parent elements
         KRATOS_ERROR_IF(rNodeElementCandidates.size() == 0) <<
             "Condition " << this->Id() << " has no candidate parent elements.\n" <<
             "Check that the FindNodalNeighboursProcess has been executed ( e.g in Solver.Initialize() )";

		for (SizeType j = 0; j < rNodeElementCandidates.size(); j++){

            ElementCandidates.push_back(rNodeElementCandidates(j));
		}
	}

	std::vector<IndexType> NodeIds(TNumNodes), ElementNodeIds;
	for (SizeType i=0; i < TNumNodes; i++){
		NodeIds[i] = rGeom[i].Id();
	}

	std::sort(NodeIds.begin(), NodeIds.end());

	for (SizeType i=0; i < ElementCandidates.size(); i++)
	{
		GeometryType& rElemGeom = ElementCandidates[i].GetGeometry();
		ElementNodeIds.resize(rElemGeom.PointsNumber());

		for (SizeType j=0; j < rElemGeom.PointsNumber(); j++){
			ElementNodeIds[j] = rElemGeom[j].Id();
		}

		std::sort(ElementNodeIds.begin(), ElementNodeIds.end());
		if ( std::includes(ElementNodeIds.begin(), ElementNodeIds.end(), NodeIds.begin(), NodeIds.end()) )
		{
            // std::cout << "Parent was found " << i << std::endl;
			mpParentElement = &ElementCandidates[i];

            std::cout << mpParentElement->Info() << std::endl;   /// works
            KRATOS_ERROR_IF(mpParentElement == NULL) << "Parent as NULL pointer (Check2)";
			return;
		}
	}
	std::cout << "PARENT NOT FOUND : error in condition -> " << this->Id() << std::endl;
	KRATOS_THROW_ERROR(std::logic_error, "Condition cannot find parent element","");
	KRATOS_CATCH("");
}


/**
 * @see NavierStokesWallCondition::GetDofList
 */
template <>
void NavierStokesWallCondition<3,3>::GetDofList(DofsVectorType& rElementalDofList,
                                                ProcessInfo& rCurrentProcessInfo)
{
    const SizeType NumNodes = 3;
    const SizeType LocalSize = 12;

    if (rElementalDofList.size() != LocalSize)
        rElementalDofList.resize(LocalSize);

    unsigned int LocalIndex = 0;

    for (unsigned int iNode = 0; iNode < NumNodes; ++iNode)
    {
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_X);
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Y);
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Z);
        rElementalDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(PRESSURE);
    }
}

/// Computes the Gauss pt. LHS contribution
/**
* @param lhs_gauss reference to the local LHS matrix
* @param data Gauss pt. data structure
*/
template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::ComputeGaussPointLHSContribution(BoundedMatrix<double,TNumNodes*(TDim+1),TNumNodes*(TDim+1)>& lhs_gauss,
const ConditionDataStruct& data)
{
    const unsigned int LocalSize = TDim+1;
    noalias(lhs_gauss) = ZeroMatrix(TNumNodes*LocalSize,TNumNodes*LocalSize);
}

/// Computes the Gauss pt. RHS contribution
/**
* @param rhs_gauss reference to the local RHS vector
* @param data Gauss pt. data structure
*/
template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::ComputeGaussPointRHSContribution(array_1d<double,TNumNodes*(TDim+1)>& rhs_gauss,
const ConditionDataStruct& data)
{
    // Initialize the local RHS
    const unsigned int LocalSize = TDim+1;
    noalias(rhs_gauss) = ZeroVector(TNumNodes*LocalSize);

    // Gauss pt. Neumann BC contribution
    this->ComputeRHSNeumannContribution(rhs_gauss, data);

    // Gauss pt. outlet inflow prevention contribution
    if (this->Is(OUTLET))
    {
        this->ComputeRHSOutletInflowContribution(rhs_gauss, data);
    }
}

/// Computes the condition RHS Neumann BC contribution
/**
* @param rhs_gauss reference to the local RHS vector
* @param data Gauss pt. data structure
*/
template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::ComputeRHSNeumannContribution(array_1d<double,TNumNodes*(TDim+1)>& rhs_gauss,
                                                                              const ConditionDataStruct& data)
{
    const unsigned int LocalSize = TDim+1;
    const GeometryType& rGeom = this->GetGeometry();

    // Add Neumann BC contribution
    for (unsigned int i=0; i<TNumNodes; ++i)
    {
        const double pext = rGeom[i].FastGetSolutionStepValue(EXTERNAL_PRESSURE);

        for (unsigned int j=0; j<TNumNodes; ++j)
        {
            unsigned int row = j*LocalSize;
            for (unsigned int d=0; d<TDim; ++d)
            {
                rhs_gauss[row+d] -= data.wGauss*data.N[j]*data.N[i]*pext*data.Normal[d];
            }
        }
    }
}

/// Computes the condition RHS outlet inflow prevention contribution
/**
* @param rhs_gauss reference to the local RHS vector
* @param data Gauss pt. data structure
*/
template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::ComputeRHSOutletInflowContribution(array_1d<double,TNumNodes*(TDim+1)>& rhs_gauss,
                                                                                   const ConditionDataStruct& data)
{
    const unsigned int LocalSize = TDim+1;
    const GeometryType& rGeom = this->GetGeometry();

    // Compute Gauss pt. density, velocity norm and velocity projection
    double rhoGauss = 0.0;
    array_1d<double, 3> vGauss = ZeroVector(3);
    for (unsigned int i=0; i<TNumNodes; ++i)
    {
        const double& rRho = rGeom[i].FastGetSolutionStepValue(DENSITY);
        const array_1d<double, 3>& rVelNode = rGeom[i].FastGetSolutionStepValue(VELOCITY);
        rhoGauss += data.N[i]*rRho;
        vGauss += data.N[i]*rVelNode;
    }

    const double vGaussProj = inner_prod(vGauss, data.Normal);
    const double vGaussSquaredNorm = std::pow(vGauss[0],2) + std::pow(vGauss[1],2) + std::pow(vGauss[2],2);

    // Add outlet inflow prevention contribution
    const double delta = data.delta;
    const double U_0 = data.charVel;
    const double S_0 = 0.5*(1-tanh(vGaussProj/(U_0*delta)));

    for (unsigned int i=0; i<TNumNodes; ++i)
    {
        unsigned int row = i*LocalSize;
        for (unsigned int d=0; d<TDim; ++d)
        {
            rhs_gauss[row+d] += data.wGauss*data.N[i]*0.5*rhoGauss*vGaussSquaredNorm*S_0*data.Normal[d];
        }
    }
}

/// Computes the 2D condition normal
/**
* @param An reference to condition normal vector
*/
template <>
void NavierStokesWallCondition<2,2>::CalculateNormal(array_1d<double,3>& An)
{
    Geometry<Node<3> >& pGeometry = this->GetGeometry();

    An[0] =   pGeometry[1].Y() - pGeometry[0].Y();
    An[1] = - (pGeometry[1].X() - pGeometry[0].X());
    An[2] =    0.00;

}

/// Computes the 3D condition normal
/**
* @param An reference to condition normal vector
*/
template <>
void NavierStokesWallCondition<3,3>::CalculateNormal(array_1d<double,3>& An )
{
    Geometry<Node<3> >& pGeometry = this->GetGeometry();

    array_1d<double,3> v1,v2;
    v1[0] = pGeometry[1].X() - pGeometry[0].X();
    v1[1] = pGeometry[1].Y() - pGeometry[0].Y();
    v1[2] = pGeometry[1].Z() - pGeometry[0].Z();

    v2[0] = pGeometry[2].X() - pGeometry[0].X();
    v2[1] = pGeometry[2].Y() - pGeometry[0].Y();
    v2[2] = pGeometry[2].Z() - pGeometry[0].Z();

    MathUtils<double>::CrossProduct(An,v1,v2);
    An *= 0.5;
}



/**
 * @see NavierStokesWallCondition::CalculateBehrSlipLeftHandSideContribution
 * 
 * @tparam TDim 
 * @tparam TNumNodes 
 */

template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::CalculateBehrSlipLeftHandSideContribution(  MatrixType& rLeftHandSideMatrix,
                                                                                            ProcessInfo& rCurrentProcessInfo,
                                                                                            ConditionDataStruct& rDataStruct )
{
    KRATOS_TRY
    // check if already computed and returning the stored value
    // possible for static meshes - not general
    if ( mHasBehrSlipLeftHandSideMatrix == true ){
        rLeftHandSideMatrix = mLeftHandSideMatrix;
        return;
    }
    
    GeometryType& rGeom = this->GetGeometry();     // one edge or one face

    std::vector< Matrix > NodalNormals(TNumNodes);

    // Retrieving the nodal normal vectors and storing them in matrix format
	for (SizeType nnode = 0; nnode < TNumNodes; nnode++){

        NodalNormals[nnode].resize(3,1);

        double sum = 0.0;
        for (int j = 0; j < 3; j++){
            NodalNormals[nnode](j,0) = rGeom[nnode].FastGetSolutionStepValue(NORMAL)(j);
            sum += NodalNormals[nnode](j,0)*NodalNormals[nnode](j,0);
        }
        for (int j = 0; j < 3; j++){
            NodalNormals[nnode](j,0) /= sqrt(sum);
        }
    }

    // KRATOS_WATCH( NodalNormals )

    MatrixType BaseLHSMatrix = zero_matrix<double>( TNumNodes*(TNumNodes+1) , TNumNodes*(TNumNodes+1) );
    MatrixType ProjectionLHSMatrix = zero_matrix<double>( TNumNodes*(TNumNodes+1) , TNumNodes*(TNumNodes+1) );
    MatrixType BaseLHSMatrixGPcontribution = zero_matrix<double>( TNumNodes*(TNumNodes+1) , TNumNodes*(TNumNodes+1) );

    const GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(GeometryData::GI_GAUSS_2);
    const unsigned int NumGauss = IntegrationPoints.size();
    Vector GaussPtsJDet = ZeroVector(NumGauss);
    rGeom.DeterminantOfJacobian(GaussPtsJDet, GeometryData::GI_GAUSS_2);
    const MatrixType Ncontainer = rGeom.ShapeFunctionsValues(GeometryData::GI_GAUSS_2);

    // Loop on gauss points
    if ( TNumNodes == 2 ){
        for(unsigned int igauss = 0; igauss < NumGauss; igauss++) {
        
            BaseLHSMatrixGPcontribution = ZeroMatrix(6, 6);
            array_1d<double, TNumNodes> N = row(Ncontainer, igauss);
            const double J = GaussPtsJDet[igauss];
            const double wGauss = J * IntegrationPoints[igauss].Weight();

            BaseLHSMatrixGPcontribution(0,2) = rDataStruct.Normal[0] *N [0] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(1,2) = rDataStruct.Normal[1] *N [0] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(3,2) = rDataStruct.Normal[0] *N [0] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(4,2) = rDataStruct.Normal[1] *N [0] * N[1] * wGauss;
  
            BaseLHSMatrixGPcontribution(0,5) = rDataStruct.Normal[0] *N [1] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(1,5) = rDataStruct.Normal[1] *N [1] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(3,5) = rDataStruct.Normal[0] *N [1] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(4,5) = rDataStruct.Normal[1] *N [1] * N[1] * wGauss;
 
                BaseLHSMatrix += BaseLHSMatrixGPcontribution;
        }
    } else if (TNumNodes == 3){
        for( unsigned int igauss = 0; igauss < NumGauss; igauss++){
            
            BaseLHSMatrixGPcontribution = ZeroMatrix(12, 12);
            array_1d<double, TNumNodes> N = row(Ncontainer, igauss);
            const double J = GaussPtsJDet[igauss];
            const double wGauss = J * IntegrationPoints[igauss].Weight();

            BaseLHSMatrixGPcontribution(0,3) = rDataStruct.Normal[0] * N[0] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(1,3) = rDataStruct.Normal[1] * N[0] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(2,3) = rDataStruct.Normal[2] * N[0] * N[0] * wGauss;

            BaseLHSMatrixGPcontribution(4,3) = rDataStruct.Normal[0] * N[0] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(5,3) = rDataStruct.Normal[1] * N[0] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(6,3) = rDataStruct.Normal[2] * N[0] * N[1] * wGauss;

            BaseLHSMatrixGPcontribution(8,3) = rDataStruct.Normal[0] * N[0] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(9,3) = rDataStruct.Normal[1] * N[0] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(10,3)= rDataStruct.Normal[2] * N[0] * N[2] * wGauss;
            // ----------------------------------------------------------------------------
            BaseLHSMatrixGPcontribution(0,7) = rDataStruct.Normal[0] * N[1] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(1,7) = rDataStruct.Normal[1] * N[1] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(2,7) = rDataStruct.Normal[2] * N[1] * N[0] * wGauss;

            BaseLHSMatrixGPcontribution(4,7) = rDataStruct.Normal[0] * N[1] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(5,7) = rDataStruct.Normal[1] * N[1] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(6,7) = rDataStruct.Normal[2] * N[1] * N[1] * wGauss;

            BaseLHSMatrixGPcontribution(8,7) = rDataStruct.Normal[0] * N[1] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(9,7) = rDataStruct.Normal[1] * N[1] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(10,7)= rDataStruct.Normal[2] * N[1] * N[2] * wGauss;
            // ----------------------------------------------------------------------------
            BaseLHSMatrixGPcontribution(0,11) = rDataStruct.Normal[0] * N[2] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(1,11) = rDataStruct.Normal[1] * N[2] * N[0] * wGauss;
            BaseLHSMatrixGPcontribution(2,11) = rDataStruct.Normal[2] * N[2] * N[0] * wGauss;
   
            BaseLHSMatrixGPcontribution(4,11) = rDataStruct.Normal[0] * N[2] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(5,11) = rDataStruct.Normal[1] * N[2] * N[1] * wGauss;
            BaseLHSMatrixGPcontribution(6,11) = rDataStruct.Normal[2] * N[2] * N[1] * wGauss;
   
            BaseLHSMatrixGPcontribution(8,11) = rDataStruct.Normal[0] * N[2] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(9,11) = rDataStruct.Normal[1] * N[2] * N[2] * wGauss;
            BaseLHSMatrixGPcontribution(10,11)= rDataStruct.Normal[2] * N[2] * N[2] * wGauss;

            BaseLHSMatrix += BaseLHSMatrixGPcontribution;
        }
    }

    // KRATOS_WATCH( BaseLHSMatrix )

    // Computation of NodalMultMatrix = ( [I] - (na)(na) ) for each node
    std::vector<MatrixType> NodalMultMatrix(TNumNodes);
    const MatrixType auxIdentMatrix = identity_matrix<double>(3);
    MatrixType auxMatrix = zero_matrix<double>(3,3);
    
    for (SizeType nnode=0; nnode < TNumNodes; nnode++){
        auxMatrix = prod( NodalNormals[nnode], trans(NodalNormals[nnode]) );
        NodalMultMatrix[nnode] = auxIdentMatrix - auxMatrix;
    }

    // KRATOS_WATCH( NodalMultMatrix )

    for(unsigned int nnode = 0; nnode < TNumNodes; nnode++){
        for( unsigned int i = 0; i < 3; i++){
            for( unsigned int j = 0; j < 3; j++){

                unsigned int istart = nnode * (TNumNodes+1);
                unsigned int jstart = nnode * (TNumNodes+1);
                ProjectionLHSMatrix(istart + i, jstart + j) = NodalMultMatrix[nnode](i,j);

            }
        }
    }

    mLeftHandSideMatrix.resize( TNumNodes*(TNumNodes+1), TNumNodes*(TNumNodes+1) );

    mLeftHandSideMatrix = prod( ProjectionLHSMatrix, BaseLHSMatrixGPcontribution );
    rLeftHandSideMatrix = prod( ProjectionLHSMatrix, BaseLHSMatrixGPcontribution );

    KRATOS_CATCH("");

}

/**
 * @see NavierStokesWallCondition::CalculateBehrSlipRightHandSideContribution
 * 
 * @tparam TDim 
 * @tparam TNumNodes 
 */

template<unsigned int TDim, unsigned int TNumNodes>
void NavierStokesWallCondition<TDim,TNumNodes>::CalculateBehrSlipRightHandSideContribution( VectorType& rRightHandSideVector,
                                                                                            ProcessInfo& rCurrentProcessInfo,
                                                                                            ConditionDataStruct& rDataStruct )
{
    KRATOS_TRY

    GeometryType& rGeom = this->GetGeometry();

    std::vector<IndexType> NodeIds(TNumNodes), ElementNodeIds;
    std::vector< Matrix > NodalNormals(TNumNodes);
    std::vector< Matrix > NodalEntriesRHS(TNumNodes);
    Matrix sigmaVoigtMatrix;
    Matrix stressTimesNormal;

    // Retrieve the nodal normal vectors, normalize, and store them in matrix
	for (SizeType nnode=0; nnode < TNumNodes; nnode++){
		NodeIds[nnode] = rGeom[nnode].Id();
        NodalNormals[nnode].resize(3,1);

        double sumOfSquares = 0.0;
        for (int j = 0; j < 3; j++)
        {
            NodalNormals[nnode](j,0) = rGeom[nnode].FastGetSolutionStepValue(NORMAL)(j);
            sumOfSquares += NodalNormals[nnode](j,0) * NodalNormals[nnode](j,0);
        }
        // normalization
        for (int j = 0; j < 3; j++)
        {
            NodalNormals[nnode](j,0) /= sqrt(sumOfSquares);
        }
	}

    // Computation of NodalMultMatrix = ( [I] - (na)(na) ) (for all nodes)
    std::vector<MatrixType> NodalProjectionMatrix(TNumNodes);
    const MatrixType auxIdentMatrix = identity_matrix<double>(3);
    MatrixType auxMatrix = zero_matrix<double>(3,3);
    
    for (SizeType nnode=0; nnode < TNumNodes; nnode++){
        auxMatrix = prod( NodalNormals[nnode], trans(NodalNormals[nnode]) );
        NodalProjectionMatrix[nnode] = auxIdentMatrix - auxMatrix;
    }

    // FluidElementUtilities<NumNodes>::VoigtTransformForProduct(rUnitNormal, voigt_normal_projection_matrix)
    MatrixType normalVoigtMatrix = zero_matrix<double>(3,3);

    if ( TNumNodes == 2 ){
        if (normalVoigtMatrix.size2() != 3){
            normalVoigtMatrix.resize( 3, 3, false );
        }
        normalVoigtMatrix = zero_matrix<double>(3,3);
        sigmaVoigtMatrix = zero_matrix<double>(3,1);
        normalVoigtMatrix(0,0) = rDataStruct.Normal(0);
        normalVoigtMatrix(1,1) = rDataStruct.Normal(1);
        normalVoigtMatrix(0,2) = rDataStruct.Normal(1);
        normalVoigtMatrix(1,2) = rDataStruct.Normal(0);
    } else if ( TNumNodes == 3 ){
        if (normalVoigtMatrix.size2() != 6){
            normalVoigtMatrix.resize( 3, 6, false );
        }
        normalVoigtMatrix = zero_matrix<double>(3,6);
        sigmaVoigtMatrix = zero_matrix<double>(6,1);
        normalVoigtMatrix(0,0) = rDataStruct.Normal(0);
        normalVoigtMatrix(1,1) = rDataStruct.Normal(1);
        normalVoigtMatrix(2,2) = rDataStruct.Normal(2);
        normalVoigtMatrix(0,3) = rDataStruct.Normal(1);
        normalVoigtMatrix(0,5) = rDataStruct.Normal(2);
        normalVoigtMatrix(1,3) = rDataStruct.Normal(0);
        normalVoigtMatrix(1,4) = rDataStruct.Normal(2);
        normalVoigtMatrix(2,4) = rDataStruct.Normal(1);
        normalVoigtMatrix(2,5) = rDataStruct.Normal(0);
    } 

    // checking for existance of the parent
    if (mpParentElement != NULL){
        // std::cout << mpParentElement->Info() << std::endl;    ////   PROBLEM
    } else {
        // std::cout << "NULL was found..." << std::endl;
    }
    
    VectorType ShearStressOfElement(3, 0.0);
    ShearStressOfElement = ZeroVector(3);

    // linearization in terms of the ShearStress ( not considered as f(u^{i+1}) )
    if (TNumNodes == 3 ){
        ShearStressOfElement.resize(6, false);
        ShearStressOfElement = ZeroVector(6);
    }
    
    mpParentElement->Calculate( FLUID_STRESS, ShearStressOfElement, rCurrentProcessInfo );

    // adding PRESSURE to the viscous shear stresses
    std::vector< Matrix > CompleteNodalSigma(TNumNodes);

    if ( TNumNodes == 2 ){
        for (unsigned int nnode = 0; nnode < TNumNodes; nnode++){
            if ( CompleteNodalSigma[nnode].size1() != 3 ){
                CompleteNodalSigma[nnode].resize(3,1);
            }
            CompleteNodalSigma[nnode](0,0) = ShearStressOfElement[0] - rGeom[nnode].FastGetSolutionStepValue(PRESSURE);
            CompleteNodalSigma[nnode](1,0) = ShearStressOfElement[1] - rGeom[nnode].FastGetSolutionStepValue(PRESSURE);
            CompleteNodalSigma[nnode](2,0) = ShearStressOfElement[2]; // no pressure in shear component
        }
    } else if ( TNumNodes == 3 ){
        for (unsigned int nnode = 0; nnode < TNumNodes; nnode++){
            if ( CompleteNodalSigma[nnode].size1() != 6 ){
                CompleteNodalSigma[nnode].resize(6,1);
            }
            CompleteNodalSigma[nnode](0,0) = ShearStressOfElement[0] - rGeom[nnode].FastGetSolutionStepValue(PRESSURE);
            CompleteNodalSigma[nnode](1,0) = ShearStressOfElement[1] - rGeom[nnode].FastGetSolutionStepValue(PRESSURE);
            CompleteNodalSigma[nnode](2,0) = ShearStressOfElement[2] - rGeom[nnode].FastGetSolutionStepValue(PRESSURE);
            CompleteNodalSigma[nnode](3,0) = ShearStressOfElement[3];  // no pressure in shear component
            CompleteNodalSigma[nnode](4,0) = ShearStressOfElement[4];  // no pressure in shear component
            CompleteNodalSigma[nnode](5,0) = ShearStressOfElement[5];  // no pressure in shear component
        }
    }

    // retrieving Gauss integration point data
    const GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(GeometryData::GI_GAUSS_2);
    const unsigned int NumGauss = IntegrationPoints.size();
    Vector GaussPtsJDet = ZeroVector(NumGauss);
    rGeom.DeterminantOfJacobian(GaussPtsJDet, GeometryData::GI_GAUSS_2);
    const MatrixType Ncontainer = rGeom.ShapeFunctionsValues(GeometryData::GI_GAUSS_2);

    Matrix CompleteSigmaInterpolated;
    CompleteSigmaInterpolated.resize(3,1);

    // Loop on nodes
    for (unsigned int nnode = 0; nnode < TNumNodes; nnode++){

        if (NodalEntriesRHS[nnode].size1() != TNumNodes+1){
            NodalEntriesRHS[nnode].resize(TNumNodes+1, 1);
        }

        NodalEntriesRHS[nnode] = zero_matrix<double>(3, 1);
        
        // Loop on gauss points
        for(unsigned int igauss = 0; igauss < NumGauss; igauss++){

            array_1d<double, TNumNodes> N = row(Ncontainer, igauss);
            const double J = GaussPtsJDet[igauss];
            const double wGauss = J * IntegrationPoints[igauss].Weight();

            // interpolation
            if (TNumNodes == 2){
                CompleteSigmaInterpolated = N[0] * prod( normalVoigtMatrix, CompleteNodalSigma[0] ) + 
                                            N[1] * prod( normalVoigtMatrix, CompleteNodalSigma[1] );
            } else if (TNumNodes == 3){
                CompleteSigmaInterpolated = N[0] * prod( normalVoigtMatrix, CompleteNodalSigma[0] ) +
                                            N[1] * prod( normalVoigtMatrix, CompleteNodalSigma[1] ) +
                                            N[2] * prod( normalVoigtMatrix, CompleteNodalSigma[2] );
            }
            
            NodalEntriesRHS[nnode] += ( wGauss * N(nnode) * CompleteSigmaInterpolated );
        }

        NodalEntriesRHS[nnode] = prod( NodalProjectionMatrix[nnode], NodalEntriesRHS[nnode] );
    }

    for (unsigned int entry = 0; entry < 3; entry++){

        for (unsigned int node = 0; node < TNumNodes; node++){
            rRightHandSideVector(entry + node*(TNumNodes+1) ) = NodalEntriesRHS[node](entry,0);
        }
    }

    KRATOS_CATCH("")
}


template class NavierStokesWallCondition<2,2>;
template class NavierStokesWallCondition<3,3>;

} // namespace Kratos
