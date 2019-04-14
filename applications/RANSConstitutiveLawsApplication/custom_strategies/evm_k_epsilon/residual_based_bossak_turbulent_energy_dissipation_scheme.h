//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//                   Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

#if !defined(KRATOS_RESIDUAL_BASED_BOSSAK_TURBULENT_ENERGY_DISSIPATION_RATE_SCHEME_H_INCLUDED)
#define KRATOS_RESIDUAL_BASED_BOSSAK_TURBULENT_ENERGY_DISSIPATION_RATE_SCHEME_H_INCLUDED

// System includes

// Project includes
#include "includes/model_part.h"
#include "solving_strategies/schemes/residual_based_bossak_velocity_scheme.h"

// Application includes
#include "custom_elements/evm_k_epsilon/evm_k_epsilon_utilities.h"
#include "custom_utilities/rans_calculation_utilities.h"
#include "rans_constitutive_laws_application_variables.h"

namespace Kratos
{
template <class TSparseSpace, class TDenseSpace>
class ResidualBasedBossakTurbulentEnergyDissipationRateScheme
    : public ResidualBasedBossakVelocityScheme<TSparseSpace, TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{

    KRATOS_CLASS_POINTER_DEFINITION(ResidualBasedBossakTurbulentEnergyDissipationRateScheme);

    typedef Node<3> NodeType;

    typedef ResidualBasedBossakVelocityScheme<TSparseSpace, TDenseSpace> BaseType;

    typedef typename BaseType::SystemMatrixType SystemMatrixType;

    typedef typename BaseType::SystemVectorType SystemVectorType;

    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;

    typedef typename BaseType::DofsArrayType DofsArrayType;

    /// Constructor.

    ResidualBasedBossakTurbulentEnergyDissipationRateScheme(const double AlphaBossak)
        : ResidualBasedBossakVelocityScheme<TSparseSpace, TDenseSpace>(AlphaBossak, {}, {&TURBULENT_ENERGY_DISSIPATION_RATE}, {&TURBULENT_ENERGY_DISSIPATION_RATE_2}, {}, {}, {})
    {
    }

    void Update(ModelPart& rModelPart,
                DofsArrayType& rDofSet,
                SystemMatrixType& rA,
                SystemVectorType& rDx,
                SystemVectorType& rb) override
    {
        KRATOS_TRY;

        BaseType::Update(rModelPart, rDofSet, rA, rDx, rb);

        // Updating the auxiliary variables
        const int number_of_nodes = rModelPart.NumberOfNodes();
#pragma omp parallel for
        for (int iNode = 0; iNode < number_of_nodes; ++iNode)
        {
            NodeType& r_node = *(rModelPart.NodesBegin() + iNode);
            const double epsilon_dot_old = r_node.FastGetSolutionStepValue(
                TURBULENT_ENERGY_DISSIPATION_RATE_2, 1);
            const double epsilon_dot = r_node.FastGetSolutionStepValue(
                TURBULENT_ENERGY_DISSIPATION_RATE_2, 0);

            r_node.FastGetSolutionStepValue(RANS_AUXILIARY_VARIABLE_2) =
                this->mAlphaBossak * epsilon_dot_old + (1.0 - this->mAlphaBossak) * epsilon_dot;
        }

        KRATOS_CATCH("");
    }

    ///@}
private:
    double mPreviousStabilizationMultiplier = 0.0;
    double mPreviousAggregatedError = 0.0;
};

} // namespace Kratos

#endif // KRATOS_RESIDUAL_BASED_BOSSAK_TURBULENT_ENERGY_DISSIPATION_RATE_SCHEME_H_INCLUDED defined