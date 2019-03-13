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
//                   Suneth Warnakulasuriya
//

#if !defined(KRATOS_TURBULENCE_EVM_K_EPSILON_PROCESS_H_INCLUDED)
#define KRATOS_TURBULENCE_EVM_K_EPSILON_PROCESS_H_INCLUDED

// System includes
#include <iostream>
#include <string>

// Project includes
#include "includes/cfd_variables.h"
#include "includes/communicator.h"
#include "includes/kratos_parameters.h"
#include "includes/model_part.h"
#include "processes/process.h"
#include "solving_strategies/builder_and_solvers/residualbased_block_builder_and_solver.h"
#include "solving_strategies/strategies/residualbased_newton_raphson_strategy.h"
#include "solving_strategies/strategies/solving_strategy.h"
#include "utilities/color_utilities.h"
#include "utilities/derivatives_extension.h"
#include "utilities/variable_utils.h"

// Application includes
#include "custom_conditions/evm_epsilon_wall_condition.h"
#include "custom_elements/evm_k_epsilon/evm_epsilon_element.h"
#include "custom_elements/evm_k_epsilon/evm_k_element.h"
#include "custom_elements/evm_k_epsilon/evm_k_epsilon_utilities.h"
#include "custom_processes/turbulence_eddy_viscosity_model_process.h"
#include "custom_strategies/evm_k_epsilon/residual_based_bossak_turbulent_energy_dissipation_scheme.h"
#include "custom_strategies/evm_k_epsilon/residual_based_bossak_turbulent_kinetic_energy_scheme.h"
#include "custom_strategies/general_convergence_criteria.h"
#include "custom_utilities/calculation_utilities.h"
#include "rans_constitutive_laws_application_variables.h"

namespace Kratos
{
///@name Kratos Classes
///@{

template <unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver>
class TurbulenceEvmKEpsilonProcess
    : public TurbulenceEddyViscosityModelProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of TurbulenceEvmKEpsilonProcess
    KRATOS_CLASS_POINTER_DEFINITION(TurbulenceEvmKEpsilonProcess);

    typedef TurbulenceEddyViscosityModelProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver> BaseType;

    typedef Node<3> NodeType;

    typedef Geometry<NodeType> GeometryType;

    typedef SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver> StrategyType;

    typedef Scheme<TSparseSpace, TDenseSpace> SchemeType;

    typedef typename SchemeType::Pointer SchemePointerType;

    typedef typename SchemeType::DofsArrayType DofsArrayType;

    typedef typename SchemeType::TSystemMatrixType SystemMatrixType;

    typedef typename SchemeType::TSystemVectorType SystemVectorType;

    typedef BuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver> BuilderAndSolverType;

    typedef typename BuilderAndSolverType::Pointer BuilderAndSolverPointerType;

    typedef ConvergenceCriteria<TSparseSpace, TDenseSpace> ConvergenceCriteriaType;

    typedef typename ConvergenceCriteriaType::Pointer ConvergenceCriteriaPointerType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor.
    TurbulenceEvmKEpsilonProcess(ModelPart& rModelPart,
                                 Parameters& rParameters,
                                 typename TLinearSolver::Pointer pDistanceLinearSolver,
                                 typename TLinearSolver::Pointer pKLinearSolver,
                                 typename TLinearSolver::Pointer pEpsilonLinearSolver)
        : TurbulenceEddyViscosityModelProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>(
              rModelPart, rParameters, pDistanceLinearSolver),
          mpKLinearSolver(pKLinearSolver),
          mpEpsilonLinearSolver(pEpsilonLinearSolver)
    {
        KRATOS_TRY

        KRATOS_INFO("TurbulenceModel")
            << "Initializing k-epsilon turbulence model" << std::endl;

        Parameters default_parameters = Parameters(R"(
        {
            "convergence_tolerances":
            {
                "k_relative_tolerance": 1e-3,
                "k_absolute_tolerance": 1e-5,
                "epsilon_relative_tolerance": 1e-3,
                "epsilon_absolute_tolerance": 1e-5,
                "turbulent_viscosity_relative_tolerance": 1e-3,
                "turbulent_viscosity_absolute_tolerance": 1e-5,
                "k_max_iterations": 10,
                "epsilon_max_iterations": 10,
                "maximum_coupling_iterations": 10,
                "echo_level": 2
            },
            "echo_level"     : 0,
            "scheme_settings": {},
            "constants":
            {
                "wall_smoothness_beta"    : 5.2,
                "von_karman"              : 0.41,
                "c_mu"                    : 0.09,
                "c1"                      : 1.44,
                "c2"                      : 1.92,
                "sigma_k"                 : 1.0,
                "sigma_epsilon"           : 1.3,
                "cross_wind_diffusion_c1" : 0.7,
                "velocity_step"           : 1
            },
            "flow_parameters":
            {
                "ramp_up_time"                : 0.03,
                "turbulent_mixing_length"     : 0.01,
                "turbulent_viscosity_fraction": 0.05,
                "free_stream_velocity"        : 1.0,
                "free_stream_k"               : 1.0,
                "free_stream_epsilon"         : 1.0,
                "velocity_ratio_constant"     : 0.003
            }
        })");

        this->mrParameters["model_properties"].ValidateAndAssignDefaults(default_parameters);
        this->mrParameters["model_properties"]["convergence_tolerances"].ValidateAndAssignDefaults(
            default_parameters["convergence_tolerances"]);
        this->mrParameters["model_properties"]["constants"].ValidateAndAssignDefaults(
            default_parameters["constants"]);
        this->mrParameters["model_properties"]["flow_parameters"].ValidateAndAssignDefaults(
            default_parameters["flow_parameters"]);

        const Parameters& model_properties =
            this->mrParameters["model_properties"]["constants"];

        rModelPart.GetProcessInfo()[WALL_SMOOTHNESS_BETA] =
            model_properties["wall_smoothness_beta"].GetDouble();
        rModelPart.GetProcessInfo()[WALL_VON_KARMAN] =
            model_properties["von_karman"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENCE_RANS_C_MU] =
            model_properties["c_mu"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENCE_RANS_C1] =
            model_properties["c1"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENCE_RANS_C2] =
            model_properties["c2"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENT_KINETIC_ENERGY_SIGMA] =
            model_properties["sigma_k"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA] =
            model_properties["sigma_epsilon"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENCE_RANS_CROSS_WIND_DIFFUSION_C1] =
            model_properties["cross_wind_diffusion_c1"].GetDouble();
        rModelPart.GetProcessInfo()[RANS_VELOCITY_STEP] =
            model_properties["velocity_step"].GetInt();

        const Parameters& flow_parameters =
            this->mrParameters["model_properties"]["flow_parameters"];
        rModelPart.GetProcessInfo()[TURBULENT_MIXING_LENGTH] =
            flow_parameters["turbulent_mixing_length"].GetDouble();
        rModelPart.GetProcessInfo()[TURBULENT_VISCOSITY_FRACTION] =
            flow_parameters["turbulent_viscosity_fraction"].GetDouble();

        mFreestreamVelocity = flow_parameters["free_stream_velocity"].GetDouble();
        mFreestreamK = flow_parameters["free_stream_k"].GetDouble();
        mFreestreamEpsilon = flow_parameters["free_stream_epsilon"].GetDouble();
        mVelocityRatio = flow_parameters["velocity_ratio_constant"].GetDouble();
        mRampUpTime = flow_parameters["ramp_up_time"].GetDouble();

        KRATOS_ERROR_IF(
            this->mrModelPart.HasSubModelPart("TurbulenceModelPartRANSEVMK"))
            << "TurbulenceEddyViscosityModelProcess: "
               "TurbulenceModelPartRANSEVMK is "
               "already found."
            << std::endl;
        this->mrModelPart.CreateSubModelPart("TurbulenceModelPartRANSEVMK");
        KRATOS_ERROR_IF(this->mrModelPart.HasSubModelPart(
            "TurbulenceModelPartRANSEVMEpsilon"))
            << "TurbulenceEddyViscosityModelProcess: "
               "TurbulenceModelPartRANSEVMEpsilon is "
               "already found."
            << std::endl;
        this->mrModelPart.CreateSubModelPart(
            "TurbulenceModelPartRANSEVMEpsilon");

        mpTurbulenceKModelPart =
            &(this->mrModelPart.GetSubModelPart("TurbulenceModelPartRANSEVMK"));
        mpTurbulenceEpsilonModelPart = &(this->mrModelPart.GetSubModelPart(
            "TurbulenceModelPartRANSEVMEpsilon"));

        KRATOS_CATCH("");
    }

    /// Destructor.
    virtual ~TurbulenceEvmKEpsilonProcess()
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    virtual void Execute() override
    {
        ProcessInfo& r_current_process_info = this->mrModelPart.GetProcessInfo();

        const int process_step = r_current_process_info[RANS_MODELLING_PROCESS_STEP];

        if (process_step == 1)
            this->AddSolutionStepVariables();
        else if (process_step == 2)
            this->AddDofs();
        else if (process_step == 3)
        {
            const double current_time = r_current_process_info[TIME];
            if (current_time < mRampUpTime)
                return;

            KRATOS_INFO_IF("TurbulenceModel", this->mEchoLevel > 0)
                << "Solving RANS equations...\n";
            this->SolveStep();
        }
        else
        {
            KRATOS_THROW_ERROR(std::runtime_error,
                               "Provided step in RANS_MODELLING_PROCESS_STEP "
                               "is not supported.",
                               "");
        }
    }

    /// this function is designed for being called at the beginning of the
    /// computations right after reading the model and the groups
    virtual void ExecuteInitialize() override
    {
        BaseType::ExecuteInitialize();

        const double nu_fraction =
            this->mrModelPart.GetProcessInfo()[TURBULENT_VISCOSITY_FRACTION];
        const int number_of_nodes = this->mrModelPart.NumberOfNodes();

#pragma omp parallel for
        for (int i = 0; i < number_of_nodes; ++i)
        {
            NodeType& r_node = *(this->mrModelPart.NodesBegin() + i);
            const double nu = r_node.FastGetSolutionStepValue(KINEMATIC_VISCOSITY);
            r_node.FastGetSolutionStepValue(TURBULENT_VISCOSITY) = nu * nu_fraction;
        }

        this->GenerateSolutionStrategies();

        mpKStrategy->Initialize();
        mpEpsilonStrategy->Initialize();
    }

    /// this function will be executed at every time step BEFORE performing the solve phase
    virtual void ExecuteInitializeSolutionStep() override
    {
        KRATOS_TRY

        BaseType::ExecuteInitializeSolutionStep();

        // if (IsBoundaryConditionsAssigned)
        // return;

        ProcessInfo& r_current_process_info = this->mrModelPart.GetProcessInfo();
        const double current_time = r_current_process_info[TIME];
        if (current_time < mRampUpTime)
            return;

        this->AssignBoundaryConditions();

        // IsBoundaryConditionsAssigned = true;

        KRATOS_CATCH("");
    }

    /// this function will be executed at every time step BEFORE performing the solve phase
    virtual void ExecuteFinalizeSolutionStep() override
    {
        KRATOS_TRY

        BaseType::ExecuteFinalizeSolutionStep();

        this->UpdateFluidViscosity();

        KRATOS_CATCH("");
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
    virtual std::string Info() const override
    {
        return std::string("TurbulenceEvmKEpsilonProcess");
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << this->Info();
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const override
    {
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected Operations
    ///@{

    void InitializeTurbulenceModelPart() override
    {
        // KRATOS_TRY;

        KRATOS_INFO("TurbulenceModel")
            << "Initializing turbulence model part\n";

        std::stringstream element_postfix;
        element_postfix << TDim << "D" << TDim + 1 << "N";

        std::stringstream condition_postfix;
        condition_postfix << TDim << "D";

        const Element& k_element =
            KratosComponents<Element>::Get("RANSEVMK" + element_postfix.str());
        const Element& epsilon_element =
            KratosComponents<Element>::Get("RANSEVMEPSILON" + element_postfix.str());
        const Condition& k_cond =
            KratosComponents<Condition>::Get("Condition" + condition_postfix.str());
        // const Condition& epsilon_cond = KratosComponents<Condition>::Get(
        //     "EpsilonWallCondition" + condition_postfix.str());
        const Condition& epsilon_cond =
            KratosComponents<Condition>::Get("Condition" + condition_postfix.str());

        this->GenerateModelPart(this->mrModelPart, *mpTurbulenceKModelPart, k_element, k_cond);
        this->GenerateModelPart(this->mrModelPart, *mpTurbulenceEpsilonModelPart,
                                epsilon_element, epsilon_cond);

        KRATOS_INFO("TurbulenceModel") << *mpTurbulenceKModelPart;
        KRATOS_INFO("TurbulenceModel") << *mpTurbulenceEpsilonModelPart;

        // KRATOS_CATCH("");
    }

    void InitializeConditions() override
    {
        KRATOS_TRY;

        this->InitializeConditionsForModelPart(mpTurbulenceKModelPart);
        this->InitializeConditionsForModelPart(mpTurbulenceEpsilonModelPart);

        KRATOS_CATCH("");
    }

    void AddSolutionStepVariables() override
    {
        KRATOS_TRY

        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(TURBULENT_KINETIC_ENERGY);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(TURBULENT_KINETIC_ENERGY_RATE);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(
            TURBULENT_ENERGY_DISSIPATION_RATE);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(
            TURBULENT_ENERGY_DISSIPATION_RATE_2);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(RANS_Y_PLUS);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(FRICTION_VELOCITY);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(TANGENTIAL_VELOCITY);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(NORMAL_VELOCITY);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(RANS_AUXILIARY_VARIABLE_1);
        this->mrModelPart.GetNodalSolutionStepVariablesList().push_back(RANS_AUXILIARY_VARIABLE_2);

        BaseType::AddSolutionStepVariables();

        KRATOS_CATCH("");
    }

    void AddDofs() override
    {
        VariableUtils().AddDof<Variable<double>>(TURBULENT_KINETIC_ENERGY, this->mrModelPart);
        VariableUtils().AddDof<Variable<double>>(
            TURBULENT_ENERGY_DISSIPATION_RATE, this->mrModelPart);
        BaseType::AddDofs();
    }
    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    std::map<std::size_t, double> mRatioTolerance;
    std::map<std::size_t, double> mAbsTolerance;

    typename TLinearSolver::Pointer mpKLinearSolver;

    typename StrategyType::Pointer mpKStrategy;

    SchemePointerType mpKScheme;

    SchemePointerType mpEpsilonScheme;

    typename TLinearSolver::Pointer mpEpsilonLinearSolver;

    typename StrategyType::Pointer mpEpsilonStrategy;

    double mFreestreamK;

    double mFreestreamEpsilon;

    double mFreestreamVelocity;

    double mVelocityRatio;

    int mMaximumCouplingIterations;

    double mTurbulentViscosityRelativeTolerance;

    double mTurbulentViscosityAbsoluteTolerance;

    double mRampUpTime;

    bool IsBoundaryConditionsAssigned = false;

    ConvergenceCriteriaPointerType mpConvergenceCriteria;

    BuilderAndSolverPointerType mpKBuilderAndSolver;

    BuilderAndSolverPointerType mpEpsilonBuilderAndSolver;

    ModelPart* mpTurbulenceKModelPart;

    ModelPart* mpTurbulenceEpsilonModelPart;

    ///@}
    ///@name Private Operators
    ///@{

    void GenerateSolutionStrategies()
    {
        KRATOS_TRY

        KRATOS_INFO("TurbulenceModel")
            << "Generating turbulence modelling strategies.\n";

        const Parameters& r_convergence_parameters =
            this->mrParameters["model_properties"]["convergence_tolerances"];

        double k_relative_tolerance =
            r_convergence_parameters["k_relative_tolerance"].GetDouble();
        double k_absolute_tolerance =
            r_convergence_parameters["k_absolute_tolerance"].GetDouble();
        int k_max_iterations = r_convergence_parameters["k_max_iterations"].GetInt();

        double epsilon_relative_tolerance =
            r_convergence_parameters["epsilon_relative_tolerance"].GetDouble();
        double epsilon_absolute_tolerance =
            r_convergence_parameters["epsilon_absolute_tolerance"].GetDouble();
        int epsilon_max_iterations =
            r_convergence_parameters["epsilon_max_iterations"].GetInt();

        this->mTurbulentViscosityRelativeTolerance =
            r_convergence_parameters["turbulent_viscosity_relative_tolerance"].GetDouble();
        this->mTurbulentViscosityAbsoluteTolerance =
            r_convergence_parameters["turbulent_viscosity_absolute_tolerance"].GetDouble();
        this->mMaximumCouplingIterations =
            r_convergence_parameters["maximum_coupling_iterations"].GetInt();

        bool CalculateReactions = false;
        bool ReformDofSet = false;

        const double alpha_bossak =
            this->mrParameters["model_properties"]["scheme_settings"]
                              ["alpha_bossak"]
                                  .GetDouble();

        // K solution strategy
        BuilderAndSolverPointerType pKBuilderAndSolver = BuilderAndSolverPointerType(
            new ResidualBasedBlockBuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver>(
                mpKLinearSolver));
        mpKBuilderAndSolver = pKBuilderAndSolver;

        mpKScheme = SchemePointerType(
            new ResidualBasedBossakTurbulentKineticEnergyScheme<TSparseSpace, TDenseSpace>(
                alpha_bossak));

        ConvergenceCriteriaPointerType pKConvergenceCriteria = ConvergenceCriteriaPointerType(
            new GeneralConvergenceCriteria<TSparseSpace, TDenseSpace>(
                k_relative_tolerance, k_absolute_tolerance));

        pKConvergenceCriteria->SetEchoLevel(
            r_convergence_parameters["echo_level"].GetInt());

        mpKStrategy = typename StrategyType::Pointer(
            new ResidualBasedNewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(
                *this->mpTurbulenceKModelPart, mpKScheme, mpKLinearSolver,
                pKConvergenceCriteria, pKBuilderAndSolver, k_max_iterations,
                CalculateReactions, ReformDofSet, this->mIsMeshMoving));

        // Epsilon solution strategy
        BuilderAndSolverPointerType pEpsilonBuilderAndSolver = BuilderAndSolverPointerType(
            new ResidualBasedBlockBuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver>(
                mpEpsilonLinearSolver));

        mpEpsilonBuilderAndSolver = pEpsilonBuilderAndSolver;

        mpEpsilonScheme = SchemePointerType(
            new ResidualBasedBossakTurbulentEnergyDissipationRateScheme<TSparseSpace, TDenseSpace>(
                alpha_bossak));

        ConvergenceCriteriaPointerType pEpsilonConvergenceCriteria =
            ConvergenceCriteriaPointerType(new GeneralConvergenceCriteria<TSparseSpace, TDenseSpace>(
                epsilon_relative_tolerance, epsilon_absolute_tolerance));

        pEpsilonConvergenceCriteria->SetEchoLevel(
            r_convergence_parameters["echo_level"].GetInt());

        mpEpsilonStrategy = typename StrategyType::Pointer(
            new ResidualBasedNewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(
                // new LineSearchStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(
                *this->mpTurbulenceEpsilonModelPart, mpEpsilonScheme, mpEpsilonLinearSolver,
                pEpsilonConvergenceCriteria, pEpsilonBuilderAndSolver, epsilon_max_iterations,
                CalculateReactions, ReformDofSet, this->mIsMeshMoving));

        mpKBuilderAndSolver->SetEchoLevel(
            this->mrParameters["model_properties"]["echo_level"].GetInt());
        mpEpsilonBuilderAndSolver->SetEchoLevel(
            this->mrParameters["model_properties"]["echo_level"].GetInt());
        mpKStrategy->SetEchoLevel(
            this->mrParameters["model_properties"]["echo_level"].GetInt());
        mpEpsilonStrategy->SetEchoLevel(
            this->mrParameters["model_properties"]["echo_level"].GetInt());

        KRATOS_CATCH("");
    }

    void AssignBoundaryConditions()
    {
        const ProcessInfo& r_current_process_info = this->mrModelPart.GetProcessInfo();

        const double mixing_length = r_current_process_info[TURBULENT_MIXING_LENGTH];
        const double C_mu = r_current_process_info[TURBULENCE_RANS_C_MU];

        // Set Boundary Conditions
        const int number_of_nodes = this->mrModelPart.NumberOfNodes();
#pragma omp parallel for
        for (int i = 0; i < number_of_nodes; i++)
        {
            ModelPart::NodeIterator i_node = this->mrModelPart.NodesBegin() + i;
            if (i_node->Is(STRUCTURE))
            {
                this->SetUpWallNode(*i_node);
                this->InitializeValues(*i_node, 0.0, 0.0);
            }
            else if (i_node->Is(INLET))
            {
                array_1d<double, 3>& velocity =
                    i_node->FastGetSolutionStepValue(VELOCITY, 0);
                const double velocity_mag = norm_2(velocity);
                const double k = velocity_mag * mVelocityRatio;
                const double epsilon = C_mu * std::pow(k, 1.5) / mixing_length;

                this->SetUpFreestreamNode(*i_node);
                this->InitializeValues(*i_node, k, epsilon);
            }
            else // internal node, set initial value
            {
                array_1d<double, 3>& velocity =
                    i_node->FastGetSolutionStepValue(VELOCITY, 0);
                const double velocity_mag = norm_2(velocity);
                const double k = std::pow(velocity_mag / mixing_length, 2);
                const double epsilon = C_mu * std::pow(k, 1.5) / mixing_length;

                this->InitializeValues(*i_node, k, epsilon);
            }
        }
    }

    void SetUpWallNode(NodeType& rNode)
    {
        rNode.Fix(TURBULENT_KINETIC_ENERGY);
        rNode.Fix(TURBULENT_ENERGY_DISSIPATION_RATE);
    }

    void SetUpFreestreamNode(NodeType& rNode)
    {
        rNode.Fix(TURBULENT_KINETIC_ENERGY);
        rNode.Fix(TURBULENT_ENERGY_DISSIPATION_RATE);
    }

    void InitializeValues(NodeType& rNode, double K, double Epsilon)
    {
        rNode.FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY) = K;
        rNode.FastGetSolutionStepValue(TURBULENT_ENERGY_DISSIPATION_RATE) = Epsilon;
    }

    void UpdateBoundaryConditions()
    {
        const ProcessInfo& r_current_process_info = this->mrModelPart.GetProcessInfo();
        const double von_karman = r_current_process_info[WALL_VON_KARMAN];
        const double beta = r_current_process_info[WALL_SMOOTHNESS_BETA];
        const double c_mu = r_current_process_info[TURBULENCE_RANS_C_MU];

        const int number_of_conditions = this->mrModelPart.NumberOfConditions();
#pragma omp parallel for
        for (int i_cond = 0; i_cond < number_of_conditions; ++i_cond)
        {
            Condition& r_condition =
                *(this->mpTurbulenceKModelPart->ConditionsBegin() + i_cond);

            // Skip the condition if it is not a wall
            if (!r_condition.Is(STRUCTURE))
                continue;

            GeometryType& r_condition_geometry = r_condition.GetGeometry();
            Element& r_element = *(r_condition.GetValue(PARENT_ELEMENT).lock());
            GeometryType& r_element_geometry = r_element.GetGeometry();

            Vector gauss_weights;
            Matrix shape_functions;
            GeometryType::ShapeFunctionsGradientsType shape_derivatives;
            CalculationUtilities::CalculateGeometryData(
                r_element_geometry, r_element.GetIntegrationMethod(),
                gauss_weights, shape_functions, shape_derivatives);

            const unsigned int number_of_condition_nodes =
                r_condition_geometry.PointsNumber();
            const std::vector<int> condition_gauss_point_indices =
                r_condition.GetValue(GAUSS_POINT_INDICES);

            // KRATOS_WATCH(condition_gauss_point_indices.size());

            for (unsigned int i_node = 0; i_node < number_of_condition_nodes; i_node++)
            {
                NodeType& r_condition_node = r_condition_geometry[i_node];

                const int gauss_index = condition_gauss_point_indices[i_node];
                const Vector& gauss_shape_functions = row(shape_functions, gauss_index);

                const double wall_distance =
                    CalculationUtilities::EvaluateInPoint<GeometryType>(
                        r_element_geometry, DISTANCE, gauss_shape_functions);
                const double nu = CalculationUtilities::EvaluateInPoint<GeometryType>(
                    r_element_geometry, KINEMATIC_VISCOSITY, gauss_shape_functions);
                const array_1d<double, 3> velocity =
                    CalculationUtilities::EvaluateInPoint<GeometryType>(
                        r_element_geometry, VELOCITY, gauss_shape_functions);
                array_1d<double, 3> normal =
                    CalculationUtilities::EvaluateInPoint<GeometryType>(
                        r_element_geometry, NORMAL, gauss_shape_functions);

                normal /= norm_2(normal);
                const double tangential_velocity =
                    std::sqrt(inner_prod(velocity, velocity) -
                              std::pow(inner_prod(velocity, normal), 2));

                // Applying boundary conditions
                r_condition_node.SetLock();
                double& y_plus = r_condition_node.FastGetSolutionStepValue(RANS_Y_PLUS);
                double& u_tangent =
                    r_condition_node.FastGetSolutionStepValue(TANGENTIAL_VELOCITY);
                double& u_normal =
                    r_condition_node.FastGetSolutionStepValue(NORMAL_VELOCITY);
                double& u_tau = r_condition_node.FastGetSolutionStepValue(FRICTION_VELOCITY);
                double& tke = r_condition_node.FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY);
                double& epsilon = r_condition_node.FastGetSolutionStepValue(
                    TURBULENT_ENERGY_DISSIPATION_RATE);

                y_plus = CalculationUtilities::CalculateYplus(
                    tangential_velocity, wall_distance, nu, von_karman, beta, 100);
                u_tangent = tangential_velocity;
                u_normal = inner_prod(velocity, normal);
                u_tau = y_plus * nu / wall_distance;
                tke = std::pow(u_tau, 2) / std::sqrt(c_mu);
                epsilon = std::pow(u_tau, 4) / (von_karman * y_plus * nu);
                r_condition_node.UnSetLock();
            }
        }
    }

    void SolveStep()
    {
        this->CalculateYplus();
        this->UpdateBoundaryConditions();
        this->UpdateTurbulentViscosity();

        mpKStrategy->InitializeSolutionStep();
        mpKStrategy->Predict();

        mpEpsilonStrategy->InitializeSolutionStep();
        mpEpsilonStrategy->Predict();

        const int number_of_nodes = this->mrModelPart.NumberOfNodes();
        Vector old_turbulent_viscosity = ZeroVector(number_of_nodes);
        Vector new_turbulent_viscosity = ZeroVector(number_of_nodes);

        bool is_converged = false;
        int step = 1;

        DofsArrayType dummy_dofs;
        SystemMatrixType dummy_matrix;
        SystemVectorType dummy_vector;

        while (!is_converged && (step <= mMaximumCouplingIterations))
        {
#pragma omp parallel for
            for (int i = 0; i < number_of_nodes; ++i)
            {
                old_turbulent_viscosity[i] =
                    (this->mrModelPart.NodesBegin() + i)->FastGetSolutionStepValue(TURBULENT_VISCOSITY);
            }

            KRATOS_INFO_IF("TurbulenceModel", this->mEchoLevel > 0)
                << "Solving for K...\n";
            mpKStrategy->SolveSolutionStep();
            KRATOS_INFO_IF("TurbulenceModel", this->mEchoLevel > 0)
                << "Solving for Epsilon...\n";
            mpEpsilonStrategy->SolveSolutionStep();

            CalculationUtilities::LowerBound<NodeType>(
                this->mrModelPart, TURBULENT_KINETIC_ENERGY, 1e-15);
            mpKScheme->Update(this->mrModelPart, dummy_dofs, dummy_matrix,
                              dummy_vector, dummy_vector);
            CalculationUtilities::LowerBound<NodeType>(
                this->mrModelPart, TURBULENT_ENERGY_DISSIPATION_RATE, 1e-15);
            mpEpsilonScheme->Update(this->mrModelPart, dummy_dofs, dummy_matrix,
                                    dummy_vector, dummy_vector);

            this->UpdateTurbulentViscosity();

#pragma omp parallel for
            for (int i = 0; i < number_of_nodes; ++i)
            {
                new_turbulent_viscosity[i] =
                    (this->mrModelPart.NodesBegin() + i)->FastGetSolutionStepValue(TURBULENT_VISCOSITY);
            }

            double increase_norm(0.0);

#pragma omp parallel for reduction(+ : increase_norm)
            for (int i = 0; i < number_of_nodes; ++i)
            {
                increase_norm += new_turbulent_viscosity[i] - old_turbulent_viscosity[i];
            }

            const double ratio = std::abs(increase_norm / number_of_nodes);
            increase_norm = std::abs(increase_norm);

            if (this->mEchoLevel > 0)
                std::cout << "[" << step << "] CONVERGENCE CHECK: TURBULENT_VISCOSITY: ratio = "
                          << std::scientific << ratio << "; exp.ratio = " << std::scientific
                          << mTurbulentViscosityRelativeTolerance
                          << ": abs = " << std::scientific << increase_norm
                          << "; exp.abs = " << std::scientific
                          << mTurbulentViscosityAbsoluteTolerance << std::endl;

            is_converged = (increase_norm < mTurbulentViscosityAbsoluteTolerance) ||
                           (ratio < mTurbulentViscosityRelativeTolerance);

            if (this->mEchoLevel > 0 && is_converged)
                std::cout << "[" << step << "] "
                          << "CONVERGENCE CHECK: TURBULENT_VISCOSITY: "
                             "*** CONVERGENCE IS ACHIEVED ***\n";

            step++;
        }

        if (!is_converged)
        {
            std::cout
                << "|----------------------------------------------------|"
                << std::endl;
            std::cout << "|    " << BOLDFONT(FRED("ATTENTION: Max coupling iterations exceeded"))
                      << "     |" << std::endl;
            std::cout
                << "|----------------------------------------------------|"
                << std::endl;
        }

        mpKStrategy->FinalizeSolutionStep();
        mpEpsilonStrategy->FinalizeSolutionStep();
    }

    double CalculateNodalTurbulentViscosity(const ModelPart::NodeIterator& iNode,
                                            const double C_mu,
                                            const double nu_fraction,
                                            const unsigned int Step) const
    {
        const double tke = iNode->FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY, Step);
        const double epsilon =
            iNode->FastGetSolutionStepValue(TURBULENT_ENERGY_DISSIPATION_RATE, Step);
        const double y_plus = iNode->FastGetSolutionStepValue(RANS_Y_PLUS);
        const double f_mu = EvmKepsilonModelUtilities::CalculateFmu(y_plus);
        const double nu = iNode->FastGetSolutionStepValue(KINEMATIC_VISCOSITY, Step);
        const double nu_min = nu_fraction * nu;
        const double nu_max = nu / nu_fraction;

        const double nu_t = EvmKepsilonModelUtilities::CalculateTurbulentViscosity(
            C_mu, tke, epsilon, f_mu, nu_min, nu_max);

        return nu_t;
    }

    void UpdateTurbulentViscosity()
    {
        DofsArrayType dummy_dofs;
        SystemMatrixType dummy_matrix;
        SystemVectorType dummy_vector;

        const int number_of_nodes = this->mrModelPart.NumberOfNodes();

        const ProcessInfo& r_current_process_info = this->mrModelPart.GetProcessInfo();

        const double C_mu = r_current_process_info[TURBULENCE_RANS_C_MU];
        const double nu_fraction = r_current_process_info[TURBULENT_VISCOSITY_FRACTION];

#pragma omp parallel for
        for (int i = 0; i < number_of_nodes; i++)
        {
            ModelPart::NodeIterator iNode = this->mrModelPart.NodesBegin() + i;
            const double nu_t =
                CalculateNodalTurbulentViscosity(iNode, C_mu, nu_fraction, 0);
            iNode->FastGetSolutionStepValue(TURBULENT_VISCOSITY) = nu_t;
        }
    }

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    TurbulenceEvmKEpsilonProcess& operator=(TurbulenceEvmKEpsilonProcess const& rOther);

    /// Copy constructor.
    TurbulenceEvmKEpsilonProcess(TurbulenceEvmKEpsilonProcess const& rOther);

    ///@}

}; // Class TurbulenceEvmKEpsilonProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function
template <unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver>
inline std::istream& operator>>(
    std::istream& rIStream,
    TurbulenceEvmKEpsilonProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>& rThis);

/// output stream function
template <unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver>
inline std::ostream& operator<<(
    std::ostream& rOStream,
    const TurbulenceEvmKEpsilonProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

} // namespace Kratos.

#endif // KRATOS_TURBULENCE_EVM_K_EPSILON_PROCESS_H_INCLUDED  defined
