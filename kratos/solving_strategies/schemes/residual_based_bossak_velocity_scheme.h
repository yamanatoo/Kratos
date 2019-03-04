//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:   Jordi Cotela
//                  Suneth Warnakulasuriya
//

#if !defined(KRATOS_RESIDUAL_BASED_BOSSAK_VELOCITY_SCHEME_H_INCLUDED)
#define KRATOS_RESIDUAL_BASED_BOSSAK_VELOCITY_SCHEME_H_INCLUDED

// System includes
#include <vector>

// External includes

// Project includes
#include "includes/checks.h"
#include "includes/define.h"
#include "includes/kratos_parameters.h"
#include "solving_strategies/schemes/scheme.h"
#include "utilities/derivatives_extension.h"
#include "utilities/indirect_scalar.h"

namespace Kratos
{
///@name Kratos Classes
///@{

/// A scheme for dynamic adjoint equations, using Bossak time integration.
/**
 * It can be used for either first- or second-order time derivatives. Elements
 * and conditions must provide a specialization of AdjointExtensions via their
 * data value container, which allows the scheme to operate independently of
 * the variable arrangements in the element or condition.
 */
template <class TSparseSpace, class TDenseSpace>
class ResidualBasedBossakVelocityScheme : public Scheme<TSparseSpace, TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{

    KRATOS_CLASS_POINTER_DEFINITION(ResidualBasedBossakVelocityScheme);

    typedef Scheme<TSparseSpace, TDenseSpace> BaseType;

    typedef typename BaseType::TSystemMatrixType SystemMatrixType;

    typedef typename BaseType::TSystemVectorType SystemVectorType;

    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;

    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    typedef typename BaseType::DofsArrayType DofsArrayType;

    typedef std::vector<Dof<double>::Pointer> DofsVectorType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor.

    ResidualBasedBossakVelocityScheme(const double AlphaBossak,
                                      const bool UpdateAcceleration = true,
                                      const bool UpdateDisplacement = true)
    {
        mAlphaBossak = AlphaBossak;
        mUpdateAcceleration = UpdateAcceleration;
        mUpdateDisplacement = UpdateDisplacement;

        KRATOS_INFO("ResidualBasedBossakVelocityScheme")
            << " Using bossak velocity scheme with alpha_bossak = " << std::scientific
            << mAlphaBossak << " [UpdateAcceleration: " << mUpdateAcceleration
            << ", UpdateDisplacement: " << mUpdateDisplacement << "]\n";

        // Allocate auxiliary memory.
        int num_threads = OpenMPUtils::GetNumThreads();

        mMassMatrix.resize(num_threads);
        mDampingMatrix.resize(num_threads);
        mValuesVector.resize(num_threads);
        mSecondDerivativeValuesVector.resize(num_threads);
        mSecondDerivativeValuesVectorOld.resize(num_threads);
    }

    /// Destructor.
    ~ResidualBasedBossakVelocityScheme() override
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    void Initialize(ModelPart& rModelPart) override
    {
        KRATOS_TRY;

        BaseType::Initialize(rModelPart);

        InitializeNodeNeighbourCount(rModelPart.Nodes());

        KRATOS_CATCH("");
    }

    void InitializeSolutionStep(ModelPart& rModelPart,
                                SystemMatrixType& rA,
                                SystemVectorType& rDx,
                                SystemVectorType& rb) override
    {
        KRATOS_TRY;

        BaseType::InitializeSolutionStep(rModelPart, rA, rDx, rb);

        const auto& r_current_process_info = rModelPart.GetProcessInfo();

        const double delta_time = r_current_process_info[DELTA_TIME];

        if (delta_time == 0)
            KRATOS_THROW_ERROR(
                std::logic_error,
                "detected delta_time = 0 in the Bossak Scheme ... check if the "
                "time step is created correctly for the current model part",
                "");

        ResidualBasedBossakVelocityScheme::CalculateBossakConstants(
            mBossak, mAlphaBossak, delta_time);

        this->CalculateNodeNeighbourCount(rModelPart);

        KRATOS_CATCH("");
    }

    void Update(ModelPart& rModelPart,
                DofsArrayType& rDofSet,
                SystemMatrixType& rA,
                SystemVectorType& rDx,
                SystemVectorType& rb) override
    {
        KRATOS_TRY;

        mpDofUpdater->UpdateDofs(rDofSet, rDx);

        this->UpdateTimeSchemeVariables(rModelPart);

        KRATOS_CATCH("");
    }

    // predicts the solution at the current step as
    // v = vold
    void Predict(ModelPart& rModelPart,
                 DofsArrayType& rDofSet,
                 SystemMatrixType& A,
                 SystemVectorType& Dv,
                 SystemVectorType& b) override
    {
        //         ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        //         const int number_of_nodes = rModelPart.NumberOfElements();

        //         std::vector<IndirectScalar<double>> current_acceleration;
        //         std::vector<IndirectScalar<double>> current_velocity;
        //         std::vector<IndirectScalar<double>> current_displacement;
        //         std::vector<IndirectScalar<double>> old_displacement;
        //         std::vector<IndirectScalar<double>> old_velocity;
        //         std::vector<IndirectScalar<double>> old_acceleration;
        //         std::vector<Dof<double>::Pointer> current_velocity_dofs;

        // #pragma omp parallel for private(current_velocity, old_velocity, current_velocity_dofs)
        //         for (int i = 0; i < number_of_nodes; ++i)
        //         {
        //             Node<3>& r_current_node = *(rModelPart.NodesBegin() + i);
        //             DerivativesExtension& r_extensions =
        //                 *r_current_node.GetValue(DERIVATIVES_EXTENSION);

        //             r_extensions.GetFirstDerivativesVector(current_velocity, 0, r_current_process_info);
        //             r_extensions.GetFirstDerivativesVector(old_velocity, 1, r_current_process_info);
        //             r_extensions.GetFirstDerivativesDofsVector(current_velocity_dofs,
        //                                                        r_current_process_info);

        //             for (unsigned int j = 0; j < current_velocity_dofs.size(); ++j)
        //             {
        //                 if (r_current_node.HasDofFor(current_velocity_dofs[j]->GetVariable()))
        //                     if (current_velocity_dofs[j]->IsFree())
        //                     {
        //                         current_velocity[j] = old_velocity[j];
        //                     }
        //             }
        //         }

        //         UpdateAcceleration();
        //         UpdateDisplacement();

        //         if (mUpdateAcceleration)
        //         {
        // #pragma omp parallel for private(current_velocity, old_velocity,
        //                                  current_acceleration, old_acceleration)
        //             for (int i = 0; i < number_of_nodes; ++i)
        //             {
        //                 Node<3>& r_current_node = *(rModelPart.NodesBegin() + i);
        //                 DerivativesExtension& r_extensions =
        //                     *r_current_node.GetValue(DERIVATIVES_EXTENSION);

        //                 r_extensions.GetSecondDerivativesVector(current_acceleration, 0,
        //                                                         r_current_process_info);
        //                 r_extensions.GetSecondDerivativesVector(old_acceleration, 1,
        //                                                         r_current_process_info);
        //                 r_extensions.GetFirstDerivativesVector(current_velocity, 0,
        //                                                        r_current_process_info);
        //                 r_extensions.GetFirstDerivativesVector(old_velocity, 1, r_current_process_info);

        //                 // Update the acceleration
        //                 for (unsigned int j = 0; j < current_acceleration.size(); ++j)
        //                     UpdateAcceleration(current_acceleration[j], current_velocity[j],
        //                                        old_velocity[j], old_acceleration[j]);
        //             }
        //         }

        //         if (mUpdateDisplacement)
        //         {
        // #pragma omp parallel for private(current_displacement, current_acceleration,
        //                                  old_displacement, old_velocity, old_acceleration)
        //             for (int i = 0; i < number_of_nodes; i++)
        //             {
        //                 Node<3>& r_node = *(rModelPart.NodesBegin() + i);
        //                 DerivativesExtension& r_extension = *r_node.GetValue(DERIVATIVES_EXTENSION);

        //                 r_extension.GetZeroDerivativesVector(current_displacement, 0,
        //                                                      r_current_process_info);
        //                 r_extension.GetZeroDerivativesVector(old_displacement, 1, r_current_process_info);
        //                 r_extension.GetSecondDerivativesVector(current_acceleration, 0,
        //                                                        r_current_process_info);
        //                 r_extension.GetSecondDerivativesVector(old_acceleration, 1,
        //                                                        r_current_process_info);
        //                 r_extension.GetFirstDerivativesVector(old_velocity, 1, r_current_process_info);

        //                 // Update the displacement
        //                 for (unsigned int j = 0; j < current_displacement.size(); ++j)
        //                     UpdateDisplacement(current_displacement[j], old_displacement[j],
        //                                        old_velocity[j], old_acceleration[j],
        //                                        current_acceleration[j]);
        //             }
        //         }
    }

    void CalculateSystemContributions(Element::Pointer pCurrentElement,
                                      LocalSystemMatrixType& rLHS_Contribution,
                                      LocalSystemVectorType& rRHS_Contribution,
                                      Element::EquationIdVectorType& rEquationId,
                                      ProcessInfo& rCurrentProcessInfo) override
    {
        KRATOS_TRY;

        const int k = OpenMPUtils::ThisThread();

        (pCurrentElement)->InitializeNonLinearIteration(rCurrentProcessInfo);
        (pCurrentElement)->CalculateLocalSystem(rLHS_Contribution, rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentElement)->CalculateLocalVelocityContribution(mDampingMatrix[k], rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentElement)->CalculateMassMatrix(mMassMatrix[k], rCurrentProcessInfo);

        AddDynamicsToLHS(rLHS_Contribution, mDampingMatrix[k], mMassMatrix[k],
                         rCurrentProcessInfo);
        AddDynamicsToRHS(pCurrentElement, rRHS_Contribution, mDampingMatrix[k],
                         mMassMatrix[k], rCurrentProcessInfo);

        (pCurrentElement)->EquationIdVector(rEquationId, rCurrentProcessInfo);

        KRATOS_CATCH("");
    }

    void Calculate_RHS_Contribution(Element::Pointer pCurrentElement,
                                    LocalSystemVectorType& rRHS_Contribution,
                                    Element::EquationIdVectorType& rEquationId,
                                    ProcessInfo& rCurrentProcessInfo) override
    {
        int k = OpenMPUtils::ThisThread();

        // Initializing the non linear iteration for the current element
        (pCurrentElement)->InitializeNonLinearIteration(rCurrentProcessInfo);

        // basic operations for the element considered
        (pCurrentElement)->CalculateRightHandSide(rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentElement)->CalculateLocalVelocityContribution(mDampingMatrix[k], rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentElement)->EquationIdVector(rEquationId, rCurrentProcessInfo);

        // adding the dynamic contributions (static is already included)
        (pCurrentElement)->CalculateMassMatrix(mMassMatrix[k], rCurrentProcessInfo);
        AddDynamicsToRHS(pCurrentElement, rRHS_Contribution, mDampingMatrix[k],
                         mMassMatrix[k], rCurrentProcessInfo);
    }

    void Condition_CalculateSystemContributions(Condition::Pointer pCurrentCondition,
                                                LocalSystemMatrixType& rLHS_Contribution,
                                                LocalSystemVectorType& rRHS_Contribution,
                                                Condition::EquationIdVectorType& rEquationId,
                                                ProcessInfo& rCurrentProcessInfo) override
    {
        KRATOS_TRY
        int k = OpenMPUtils::ThisThread();

        (pCurrentCondition)->InitializeNonLinearIteration(rCurrentProcessInfo);
        (pCurrentCondition)->CalculateLocalSystem(rLHS_Contribution, rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentCondition)->CalculateLocalVelocityContribution(mDampingMatrix[k], rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentCondition)->EquationIdVector(rEquationId, rCurrentProcessInfo);

        (pCurrentCondition)->CalculateMassMatrix(mMassMatrix[k], rCurrentProcessInfo);
        AddDynamicsToLHS(rLHS_Contribution, mDampingMatrix[k], mMassMatrix[k],
                         rCurrentProcessInfo);
        AddDynamicsToRHS(pCurrentCondition, rRHS_Contribution,
                         mDampingMatrix[k], mMassMatrix[k], rCurrentProcessInfo);
        KRATOS_CATCH("")
    }

    void Condition_Calculate_RHS_Contribution(Condition::Pointer pCurrentCondition,
                                              LocalSystemVectorType& rRHS_Contribution,
                                              Element::EquationIdVectorType& rEquationId,
                                              ProcessInfo& rCurrentProcessInfo) override
    {
        KRATOS_TRY;

        int k = OpenMPUtils::ThisThread();

        (pCurrentCondition)->InitializeNonLinearIteration(rCurrentProcessInfo);
        (pCurrentCondition)->CalculateRightHandSide(rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentCondition)->CalculateLocalVelocityContribution(mDampingMatrix[k], rRHS_Contribution, rCurrentProcessInfo);
        (pCurrentCondition)->EquationIdVector(rEquationId, rCurrentProcessInfo);

        // adding the dynamic contributions (static is already included)
        (pCurrentCondition)->CalculateMassMatrix(mMassMatrix[k], rCurrentProcessInfo);
        AddDynamicsToRHS(pCurrentCondition, rRHS_Contribution,
                         mDampingMatrix[k], mMassMatrix[k], rCurrentProcessInfo);

        KRATOS_CATCH("");
    }

    void Clear() override
    {
        this->mpDofUpdater->Clear();
    }

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
        return "ResidualBasedBossakVelocityScheme";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }
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

    std::vector<LocalSystemVectorType> mSecondDerivativeValuesVectorOld;
    std::vector<LocalSystemVectorType> mSecondDerivativeValuesVector;
    std::vector<LocalSystemVectorType> mValuesVector;

    std::vector<LocalSystemMatrixType> mMassMatrix;
    std::vector<LocalSystemMatrixType> mDampingMatrix;

    bool mUpdateAcceleration;
    bool mUpdateDisplacement;

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    //****************************************************************************

    /**
    Kdyn = am*M + D + a1*K
     */
    void AddDynamicsToLHS(LocalSystemMatrixType& rLHS_Contribution,
                          LocalSystemMatrixType& rDampingMatrix,
                          LocalSystemMatrixType& rMassMatrix,
                          ProcessInfo& CurrentProcessInfo)
    {
        // multipling time scheme factor
        rLHS_Contribution *= mBossak.C1;

        // adding mass contribution to the dynamic stiffness
        if (rMassMatrix.size1() != 0 && mUpdateAcceleration) // if M matrix declared
        {
            noalias(rLHS_Contribution) += mBossak.C0 * rMassMatrix;
        }

        // adding  damping contribution
        if (rDampingMatrix.size1() != 0) // if M matrix declared
        {
            noalias(rLHS_Contribution) += rDampingMatrix;
        }
    }

    //****************************************************************************

    /// Add Bossak contributions from the inertial term to the RHS vector.
    /** This essentially performs bdyn = b - M*acc for the current element.
     *  Note that viscous/pressure contributions to the RHS are expected to be added by the element itself.
     *  @param[in] rCurrentElement The fluid element we are assembling.
     *  @param[in/out] rRHS_Contribution The right hand side term where the contribution will be added.
     *  @param[in] rD The elemental velocity/pressure LHS matrix.
     *  @param[in] rM The elemental acceleration LHS matrix.
     *  @param[in] rCurrentProcessInfo ProcessInfo instance for the containing ModelPart.
     */
    void AddDynamicsToRHS(Element::Pointer rCurrentElement,
                          LocalSystemVectorType& rRHS_Contribution,
                          LocalSystemMatrixType& rDampingMatrix,
                          LocalSystemMatrixType& rMassMatrix,
                          ProcessInfo& rCurrentProcessInfo)
    {
        // adding inertia contribution
        if (rMassMatrix.size1() != 0 && mUpdateAcceleration)
        {
            int k = OpenMPUtils::ThisThread();
            rCurrentElement->GetSecondDerivativesVector(
                mSecondDerivativeValuesVector[k], 0);
            (mSecondDerivativeValuesVector[k]) *= (1.00 - mBossak.Alpha);
            rCurrentElement->GetSecondDerivativesVector(
                mSecondDerivativeValuesVectorOld[k], 1);
            noalias(mSecondDerivativeValuesVector[k]) +=
                mBossak.Alpha * mSecondDerivativeValuesVectorOld[k];
            noalias(rRHS_Contribution) -=
                prod(rMassMatrix, mSecondDerivativeValuesVector[k]);
        }
    }

    /// Add Bossak contributions from the inertial term to the RHS vector.
    /** This essentially performs bdyn = b - M*acc for the current condition.
     *  Note that viscous/pressure contributions to the RHS are expected to be added by the element condition.
     *  @param[in] rCurrentCondition The fluid condition we are assembling.
     *  @param[in/out] rRHS_Contribution The right hand side term where the contribution will be added.
     *  @param[in] rD The elemental velocity/pressure LHS matrix.
     *  @param[in] rM The elemental acceleration LHS matrix.
     *  @param[in] rCurrentProcessInfo ProcessInfo instance for the containing ModelPart.
     */
    void AddDynamicsToRHS(Condition::Pointer rCurrentCondition,
                          LocalSystemVectorType& rRHS_Contribution,
                          LocalSystemMatrixType& rDampingMatrix,
                          LocalSystemMatrixType& rMassMatrix,
                          ProcessInfo& rCurrentProcessInfo)
    {
        // adding inertia contribution
        if (rMassMatrix.size1() != 0 && mUpdateAcceleration)
        {
            int k = OpenMPUtils::ThisThread();
            rCurrentCondition->GetSecondDerivativesVector(
                mSecondDerivativeValuesVector[k], 0);
            (mSecondDerivativeValuesVector[k]) *= (1.00 - mBossak.Alpha);
            rCurrentCondition->GetSecondDerivativesVector(
                mSecondDerivativeValuesVectorOld[k], 1);
            noalias(mSecondDerivativeValuesVector[k]) +=
                mBossak.Alpha * mSecondDerivativeValuesVectorOld[k];

            noalias(rRHS_Contribution) -=
                prod(rMassMatrix, mSecondDerivativeValuesVector[k]);
        }
    }

    void UpdateTimeSchemeVariables(ModelPart& rModelPart)
    {
        KRATOS_TRY;

        UpdateAcceleration(rModelPart);
        UpdateDisplacement(rModelPart);

        KRATOS_CATCH("");
    }

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
    struct BossakConstants
    {
        double Alpha;
        double Gamma;
        double Beta;
        double C0;
        double C1;
        double C2;
        double C3;
        double C4;
        double C5;
        double C6;
    };

    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    BossakConstants mBossak;
    typename TSparseSpace::DofUpdaterPointerType mpDofUpdater =
        TSparseSpace::CreateDofUpdater();

    double mAlphaBossak;

    std::vector<std::vector<IndirectScalar<double>>> mIndirectCurrentVelocityVector;
    std::vector<std::vector<IndirectScalar<double>>> mIndirectCurrentAccelerationVector;
    std::vector<std::vector<IndirectScalar<double>>> mIndirectCurrentDisplacementVector;
    std::vector<std::vector<IndirectScalar<double>>> mIndirectOldVelocityVector;
    std::vector<std::vector<IndirectScalar<double>>> mIndirectOldAccelerationVector;
    std::vector<std::vector<IndirectScalar<double>>> mIndirectOldDisplacementVector;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateResidualLocalContributions(Element::Pointer pCurrentElement,
                                             LocalSystemMatrixType& rLHS_Contribution,
                                             LocalSystemVectorType& rRHS_Contribution,
                                             ProcessInfo& rCurrentProcessInfo)
    {
        int k = OpenMPUtils::ThisThread();
        auto& r_residual_adjoint = mValuesVector[k];
        pCurrentElement->GetValuesVector(r_residual_adjoint);
        noalias(rRHS_Contribution) -= prod(rLHS_Contribution, r_residual_adjoint);
    }

    void InitializeNodeNeighbourCount(ModelPart::NodesContainerType& rNodes)
    {
        // This loop should not be omp parallel
        // The operation is not threadsafe if the value is uninitialized
        for (auto& r_node : rNodes)
            r_node.SetValue(NUMBER_OF_NEIGHBOUR_ELEMENTS, 0.0);
    }

    void CalculateNodeNeighbourCount(ModelPart& rModelPart)
    {
        // Calculate number of neighbour elements for each node.
        const int num_nodes = rModelPart.NumberOfNodes();
#pragma omp parallel for
        for (int i = 0; i < num_nodes; ++i)
        {
            Node<3>& r_node = *(rModelPart.Nodes().begin() + i);
            r_node.SetValue(NUMBER_OF_NEIGHBOUR_ELEMENTS, 0.0);
        }

        const int num_elements = rModelPart.NumberOfElements();
#pragma omp parallel for
        for (int i = 0; i < num_elements; ++i)
        {
            Element& r_element = *(rModelPart.Elements().begin() + i);
            Geometry<Node<3>>& r_geometry = r_element.GetGeometry();
            for (unsigned j = 0; j < r_geometry.PointsNumber(); ++j)
            {
                double& r_num_neighbour =
                    r_geometry[j].GetValue(NUMBER_OF_NEIGHBOUR_ELEMENTS);
#pragma omp atomic
                r_num_neighbour += 1.0;
            }
        }

        rModelPart.GetCommunicator().AssembleNonHistoricalData(NUMBER_OF_NEIGHBOUR_ELEMENTS);
    }

    static void CalculateBossakConstants(BossakConstants& rBossakConstants,
                                         const double Alpha,
                                         const double DeltaTime)
    {
        rBossakConstants.Alpha = Alpha;
        rBossakConstants.Gamma = 0.5 - rBossakConstants.Alpha;
        rBossakConstants.Beta = 0.25 * pow((1.00 - rBossakConstants.Alpha), 2);
        rBossakConstants.C0 =
            (1.0 - rBossakConstants.Alpha) / (rBossakConstants.Gamma * DeltaTime);
        rBossakConstants.C1 =
            DeltaTime / (rBossakConstants.Beta * rBossakConstants.Gamma);
        rBossakConstants.C2 = 1.0 / (rBossakConstants.Gamma * DeltaTime);
        rBossakConstants.C3 = (1.0 - rBossakConstants.Gamma) / rBossakConstants.Gamma;
        rBossakConstants.C4 =
            std::pow(DeltaTime, 2) * (-2.0 * rBossakConstants.Beta + 1.0) / 2.0;
        rBossakConstants.C5 = std::pow(DeltaTime, 2) * rBossakConstants.Beta;
        rBossakConstants.C6 = DeltaTime;
    }

    static void AssembleVariables(const std::vector<const VariableData*>& rVariables,
                                  Communicator& rComm)
    {
        KRATOS_TRY;
        for (auto p_variable_data : rVariables)
        {
            if (KratosComponents<Variable<array_1d<double, 3>>>::Has(
                    p_variable_data->Name()))
            {
                const auto& r_variable =
                    KratosComponents<Variable<array_1d<double, 3>>>::Get(
                        p_variable_data->Name());
                rComm.AssembleCurrentData(r_variable);
            }
            else if (KratosComponents<Variable<double>>::Has(p_variable_data->Name()))
            {
                const auto& r_variable =
                    KratosComponents<Variable<double>>::Get(p_variable_data->Name());
                rComm.AssembleCurrentData(r_variable);
            }
            else
            {
                KRATOS_ERROR << "Variable \"" << p_variable_data->Name() << "\" not found!\n";
            }
        }
        KRATOS_CATCH("");
    }

    void UpdateAcceleration(ModelPart& rModelPart)
    {
        if (!mUpdateAcceleration)
            return;

        ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();
        const int number_of_elements = rModelPart.NumberOfElements();

        auto displacement_vars = GatherVariables(
            rModelPart.Elements(),
            [](const DerivativesExtension& rExtensions,
               std::vector<const VariableData*>& rVec, ProcessInfo& rCurrentProcessInfo) {
                rExtensions.GetZeroDerivativesVariables(rVec, rCurrentProcessInfo);
            }, r_current_process_info);
#pragma omp parallel for
        for (int i = 0; i < number_of_elements; ++i)
        {
            Element& r_element = *(rModelPart.ElementsBegin() + i);
            Geometry<Node<3>>& r_geometry = r_element.GetGeometry();
            DerivativesExtension& r_extensions = *r_element.GetValue(DERIVATIVES_EXTENSION);

            const int k = OpenMPUtils::ThisThread();

            for (unsigned int i_node = 0; i_node < r_geometry.PointsNumber(); ++i_node)
            {
                r_extensions.GetFirstDerivativesVector(
                    i_node, mIndirectCurrentVelocityVector[k], 0, r_current_process_info);
                r_extensions.GetFirstDerivativesVector(
                    i_node, mIndirectOldVelocityVector[k], 1, r_current_process_info);
                r_extensions.GetSecondDerivativesVector(
                    i_node, mIndirectCurrentAccelerationVector[k], 0, r_current_process_info);
                r_extensions.GetSecondDerivativesVector(
                    i_node, mIndirectOldAccelerationVector[k], 1, r_current_process_info);

                r_geometry[i_node].SetLock();
                for (unsigned int i_var = 0;
                     i_var < mIndirectCurrentAccelerationVector[k].size(); ++i_var)
                    mIndirectCurrentAccelerationVector[k][i_var] =
                        mBossak.C2 * (mIndirectCurrentVelocityVector[k][i_var] -
                                      mIndirectOldVelocityVector[k][i_var]) -
                        mBossak.C3 * mIndirectOldAccelerationVector[k][i_var];
                r_geometry[i_node].UnSetLock();
            }
        }
        AssembleVariables(displacement_vars, rModelPart.GetCommunicator());
    }

    void UpdateDisplacement(ModelPart& rModelPart)
    {
        if (!mUpdateDisplacement)
            return;

        ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();
        const int number_of_elements = rModelPart.NumberOfElements();

        auto displacement_vars = GatherVariables(
            rModelPart.Elements(),
            [](const DerivativesExtension& rExtensions,
               std::vector<const VariableData*>& rVec, ProcessInfo& rCurrentProcessInfo) {
                rExtensions.GetZeroDerivativesVariables(rVec, rCurrentProcessInfo);
            }, r_current_process_info);

#pragma omp parallel for
        for (int i = 0; i < number_of_elements; ++i)
        {
            Element& r_element = *(rModelPart.ElementsBegin() + i);
            Geometry<Node<3>>& r_geometry = r_element.GetGeometry();
            DerivativesExtension& r_extensions = *r_element.GetValue(DERIVATIVES_EXTENSION);

            const int k = OpenMPUtils::ThisThread();

            for (unsigned int i_node = 0; i_node < r_geometry.PointsNumber(); ++i_node)
            {
                r_extensions.GetZeroDerivativesVector(
                    i_node, mIndirectCurrentDisplacementVector[k], 0, r_current_process_info);
                r_extensions.GetZeroDerivativesVector(
                    i_node, mIndirectOldDisplacementVector[k], 1, r_current_process_info);
                r_extensions.GetFirstDerivativesVector(
                    i_node, mIndirectOldVelocityVector[k], 1, r_current_process_info);
                r_extensions.GetSecondDerivativesVector(
                    i_node, mIndirectCurrentAccelerationVector[k], 0, r_current_process_info);
                r_extensions.GetSecondDerivativesVector(
                    i_node, mIndirectOldAccelerationVector[k], 1, r_current_process_info);

                r_geometry[i_node].SetLock();
                for (unsigned int i_var = 0;
                     i_var < mIndirectCurrentDisplacementVector[k].size(); ++i_var)
                    mIndirectCurrentDisplacementVector[k][i_var] =
                        mIndirectOldDisplacementVector[k][i_var] +
                        mBossak.C6 * mIndirectOldVelocityVector[k][i_var] +
                        mBossak.C4 * mIndirectOldAccelerationVector[k][i_var] +
                        mBossak.C5 * mIndirectCurrentAccelerationVector[k][i_var];
                r_geometry[i_node].UnSetLock();
            }
        }
        AssembleVariables(displacement_vars, rModelPart.GetCommunicator());
    }

    struct Hash
    {
        std::size_t operator()(const VariableData* const& p) const
        {
            return p->Key();
        }
    };

    struct Pred
    {
        bool operator()(const VariableData* const l, const VariableData* const r) const
        {
            return *l == *r;
        }
    };

    // Gathers variables needed for assembly.
    static std::vector<const VariableData*> GatherVariables(
        const ModelPart::ElementsContainerType& rElements,
        std::function<void(const DerivativesExtension&, std::vector<const VariableData*>&, ProcessInfo&)> GetLocalVars, ProcessInfo& rCurrentProcessInfo)
    {
        KRATOS_TRY;
        const int num_threads = OpenMPUtils::GetNumThreads();
        std::vector<const VariableData*> local_vars;
        std::vector<std::unordered_set<const VariableData*, Hash, Pred>> thread_vars(num_threads);
#pragma omp parallel for private(local_vars)
        for (int i = 0; i < static_cast<int>(rElements.size()); ++i)
        {
            auto& r_element = *(rElements.begin() + i);
            GetLocalVars(*r_element.GetValue(DERIVATIVES_EXTENSION), local_vars, rCurrentProcessInfo);
            const int k = OpenMPUtils::ThisThread();
            thread_vars[k].insert(local_vars.begin(), local_vars.end());
        }
        std::unordered_set<const VariableData*, Hash, Pred> all_vars;
        for (int i = 0; i < num_threads; ++i)
        {
            all_vars.insert(thread_vars[i].begin(), thread_vars[i].end());
        }
        return std::vector<const VariableData*>{all_vars.begin(), all_vars.end()};
        KRATOS_CATCH("");
    }
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

}; /* Class ResidualBasedBossakVelocityScheme */

///@}

///@name Type Definitions
///@{

///@}

} /* namespace Kratos.*/

#endif /* KRATOS_RESIDUAL_BASED_BOSSAK_VELOCITY_SCHEME_H_INCLUDED defined */
