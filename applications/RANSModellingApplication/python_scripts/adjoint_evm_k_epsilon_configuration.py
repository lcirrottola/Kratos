from __future__ import print_function, absolute_import, division

import KratosMultiphysics as Kratos
import KratosMultiphysics.RANSModellingApplication as KratosRANS
from adjoint_turbulence_eddy_viscosity_model_configuration import AdjointTurbulenceEddyViscosityModelConfiguration

from KratosMultiphysics.kratos_utilities import CheckIfApplicationsAvailable
if CheckIfApplicationsAvailable("FluidDynamicsApplication"):
    import KratosMultiphysics.FluidDynamicsApplication
else:
    msg = "k-epsilon turbulence model depends on the FluidDynamicsApplication which is not found."
    msg += " Please re-install/compile with FluidDynamicsApplication"
    raise Exception(msg)

class AdjointTurbulenceKEpsilonConfiguration(
        AdjointTurbulenceEddyViscosityModelConfiguration):
    def __init__(self, model, parameters):
        super(AdjointTurbulenceKEpsilonConfiguration, self).__init__(model, parameters)

        default_settings = Kratos.Parameters(r'''{
            "echo_level"        :0,
            "constants":
            {
                "wall_smoothness_beta"    : 5.2,
                "von_karman"              : 0.41,
                "c_mu"                    : 0.09,
                "c1"                      : 1.44,
                "c2"                      : 1.92,
                "sigma_k"                 : 1.0,
                "sigma_epsilon"           : 1.3
            },
            "flow_parameters":
            {
                "ramp_up_time"                        : 0.5
            }
        }''')

        parameters["model_settings"].ValidateAndAssignDefaults(default_settings)
        self.model_settings = parameters["model_settings"]

        self.element_name = "RANSEVMMonolithicKEpsilonVMSAdjoint"

        self.ramp_up_time = self.model_settings["flow_parameters"]["ramp_up_time"].GetDouble()

    def InitializeModelConstants(self):
        # reading constants
        constants = self.model_settings["constants"]
        self.fluid_model_part.ProcessInfo[KratosRANS.WALL_SMOOTHNESS_BETA] = constants["wall_smoothness_beta"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.WALL_VON_KARMAN] = constants["von_karman"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C_MU] = constants["c_mu"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C1] = constants["c1"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C2] = constants["c2"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_KINETIC_ENERGY_SIGMA] = constants["sigma_k"].GetDouble()
        self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA] = constants["sigma_epsilon"].GetDouble()

    def PrepareSolvingStrategy(self):
        domain_size = self.fluid_model_part.ProcessInfo[Kratos.DOMAIN_SIZE]
        self.fluid_model_part.ProcessInfo[TURBULENT_KINETIC_ENERGY] = domain_size+1
        self.fluid_model_part.ProcessInfo[TURBULENT_ENERGY_DISSIPATION_RATE] = domain_size+2

        Kratos.Logger.PrintInfo(self.__class__.__name__, "All adjoint turbulence solution strategies are created.")

    def AddVariables(self):
        # adding k-epsilon specific variables
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_KINETIC_ENERGY)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_KINETIC_ENERGY_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUXILIARY_VARIABLE_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUXILIARY_VARIABLE_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_1_ADJOINT_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_1_ADJOINT_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_1_ADJOINT_3)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUX_ADJOINT_SCALAR_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_2_ADJOINT_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_2_ADJOINT_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_SCALAR_2_ADJOINT_3)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUX_ADJOINT_SCALAR_2)


        super(TurbulenceKEpsilonConfiguration, self).AddVariables()

    def AddDofs(self):
        Kratos.VariableUtils().AddDof(KratosRANS.RANS_SCALAR_1_ADJOINT_1, self.fluid_model_part)
        Kratos.VariableUtils().AddDof(KratosRANS.RANS_SCALAR_2_ADJOINT_1, self.fluid_model_part)

        Kratos.Logger.PrintInfo(self.__class__.__name__, "DOFs added successfully.")

    def Initialize(self):
        super(AdjointTurbulenceKEpsilonConfiguration, self).Initialize()
        self.InitializeModelConstants()

    def InitializeSolutionStep(self):
        if (self.fluid_model_part.ProcessInfo[KratosRANS.IS_CO_SOLVING_PROCESS_ACTIVE]):
            super(AdjointTurbulenceKEpsilonConfiguration, self).InitializeSolutionStep()

    def FinalizeSolutionStep(self):
        super(AdjointTurbulenceKEpsilonConfiguration, self).FinalizeSolutionStep()
        time = self.fluid_model_part.ProcessInfo[Kratos.TIME]
        if (time >= self.ramp_up_time):
            self.fluid_model_part.ProcessInfo[KratosRANS.IS_CO_SOLVING_PROCESS_ACTIVE] = True
