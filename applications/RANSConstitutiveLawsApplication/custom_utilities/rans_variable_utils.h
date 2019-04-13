//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//
//

#if !defined(KRATOS_RANS_VARIABLE_UTILS)
#define KRATOS_RANS_VARIABLE_UTILS

/* System includes */

/* External includes */

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "utilities/variable_utils.h"

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

/**
 * @class VariableUtils
 * @ingroup KratosCore
 * @brief This class implements a set of auxiliar, already parallelized, methods to
 * perform some common tasks related with the variable values and fixity.
 * @details The methods are exported to python in order to add this improvements to the python interface
 * @author Riccardo Rossi
 * @author Ruben Zorrilla
 * @author Vicente Mataix Ferrandiz
 */
class RansVariableUtils : public VariableUtils
{
public:
    ///@name Type Definitions
    ///@{

    /// We create the Pointer related to RansVariableUtils
    KRATOS_CLASS_POINTER_DEFINITION(RansVariableUtils);

    void FixScalarVariableDofs(const Flags& rFlag,
                               const Variable<double>& rVariable,
                               ModelPart::NodesContainerType& rNodes)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();

#pragma omp parallel for
        for (int i = 0; i < number_of_nodes; i++)
        {
            ModelPart::NodeType& r_node = *(rNodes.begin() + i);
            if (r_node.Is(rFlag))
                r_node.Fix(rVariable);
        }

        KRATOS_CATCH("")
    }

    void ClipScalarVariable(unsigned int& rNumberOfNodesBelowMinimum,
                            unsigned int& rNumberOfNodesAboveMaximum,
                            const double MinimumValue,
                            const double MaximumValue,
                            const Variable<double>& rVariable,
                            ModelPart::NodesContainerType& rNodes)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();

        rNumberOfNodesBelowMinimum = 0;
        rNumberOfNodesAboveMaximum = 0;

#pragma omp parallel for reduction( +: rNumberOfNodesBelowMinimum, rNumberOfNodesAboveMaximum)
        for (int i = 0; i < number_of_nodes; i++)
        {
            ModelPart::NodeType& r_node = *(rNodes.begin() + i);
            double& r_value = r_node.FastGetSolutionStepValue(rVariable);

            if (r_value < MinimumValue)
            {
                rNumberOfNodesBelowMinimum++;
                r_value = MinimumValue;
            }
            else if (r_value > MaximumValue)
            {
                rNumberOfNodesAboveMaximum++;
                r_value = MaximumValue;
            }
        }

        KRATOS_CATCH("")
    }

    unsigned int GetNumberOfNegativeScalarValueNodes(const ModelPart::NodesContainerType& rNodes,
                                                     const Variable<double>& rVariable)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();
        unsigned int number_of_negative_nodes = 0;

#pragma omp parallel for reduction(+ : number_of_negative_nodes)
        for (int i = 0; i < number_of_nodes; i++)
        {
            const double value = (rNodes.begin() + i)->FastGetSolutionStepValue(rVariable);
            number_of_negative_nodes += (value < 0.0);
        }

        return number_of_negative_nodes;

        KRATOS_CATCH("");
    }

    double GetMinimumScalarValue(const ModelPart::NodesContainerType& rNodes,
                                 const Variable<double>& rVariable)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();

        if (number_of_nodes == 0)
            return 0.0;

        double min_value = rNodes.begin()->FastGetSolutionStepValue(rVariable);

#pragma omp parallel for reduction(min : min_value)
        for (int i = 0; i < number_of_nodes; i++)
        {
            const double value = (rNodes.begin() + i)->FastGetSolutionStepValue(rVariable);
            min_value = std::min(min_value, value);
        }

        return min_value;

        KRATOS_CATCH("");
    }

    double GetMaximumScalarValue(const ModelPart::NodesContainerType& rNodes,
                                 const Variable<double>& rVariable)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();

        if (number_of_nodes == 0)
            return 0.0;

        double max_value = rNodes.begin()->FastGetSolutionStepValue(rVariable);

#pragma omp parallel for reduction(max : max_value)
        for (int i = 0; i < number_of_nodes; i++)
        {
            const double value = (rNodes.begin() + i)->FastGetSolutionStepValue(rVariable);
            max_value = std::max(max_value, value);
        }

        return max_value;

        KRATOS_CATCH("");
    }

    double GetScalarVariableDifferenceNormSquare(const ModelPart::NodesContainerType& rNodes,
                                                 const Variable<double>& rVariableA,
                                                 const Variable<double>& rVariableB)
    {
        KRATOS_TRY

        CheckVariableExists(rVariableA, rNodes);
        CheckVariableExists(rVariableB, rNodes);

        const int number_of_nodes = rNodes.size();
        double increase_norm_square = 0.0;

#pragma omp parallel for reduction(+ : increase_norm_square)
        for (int i = 0; i < number_of_nodes; i++)
        {
            const ModelPart::NodeType& r_node = *(rNodes.begin() + i);
            const double value_a = r_node.FastGetSolutionStepValue(rVariableA);
            const double value_b = r_node.FastGetSolutionStepValue(rVariableB);
            increase_norm_square += std::pow(value_a - value_b, 2);
        }

        return increase_norm_square;

        KRATOS_CATCH("");
    }

    double GetScalarVariableSolutionNormSquare(const ModelPart::NodesContainerType& rNodes,
                                               const Variable<double>& rVariable)
    {
        KRATOS_TRY

        CheckVariableExists(rVariable, rNodes);

        const int number_of_nodes = rNodes.size();
        double solution_norm_square = 0.0;

#pragma omp parallel for reduction(+ : solution_norm_square)
        for (int i = 0; i < number_of_nodes; i++)
        {
            const double solution_value =
                (rNodes.begin() + i)->FastGetSolutionStepValue(rVariable);
            solution_norm_square += std::pow(solution_value, 2);
        }

        return solution_norm_square;

        KRATOS_CATCH("");
    }

    void CopyNodalSolutionStepVariablesList(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart)
    {
        KRATOS_TRY

        rDestinationModelPart.GetNodalSolutionStepVariablesList() =
            rOriginModelPart.GetNodalSolutionStepVariablesList();

        KRATOS_CATCH("");
    }
}; /* Class RansVariableUtils */

///@}

///@name Type Definitions
///@{

///@}

} /* namespace Kratos.*/

#endif /* KRATOS_RANS_VARIABLE_UTILS  defined */