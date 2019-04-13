from __future__ import print_function, absolute_import, division

import KratosMultiphysics as Kratos
from python_solver import PythonSolver
from kratos_utilities import CheckIfApplicationsAvailable

if CheckIfApplicationsAvailable("RANSConstitutiveLawsApplication"):
    import KratosMultiphysics.RANSConstitutiveLawsApplication as KratosRANS


def CreateTurbulenceModel(model, settings):
    if not CheckIfApplicationsAvailable("RANSConstitutiveLawsApplication"):
        msg = "Using a turbulence model requires the RANSConstitutiveLawsApplication. "
        msg += "Please re-install/re-compile with RANSConstitutiveLawsApplication."
        raise Exception(msg)

    from turbulence_model_factory import Factory
    return Factory(model, settings)


class TurbulenceModelConfiguration(PythonSolver):
    def __init__(self, model, settings):
        super(TurbulenceModelConfiguration, self).__init__(model, settings)

        self.model_elements = []
        self.model_conditions = []
        self.turbulence_model_parts_list = []

    def CreateTurbulenceModelParts(self):
        self.domain_size = self.fluid_model_part.ProcessInfo[Kratos.
                                                             DOMAIN_SIZE]

        for element, condition in zip(self.model_elements,
                                      self.model_conditions):
            model_part_name = "TurbulenceModelPart_" + element
            if not self.model.HasModelPart(model_part_name):

                element_name = "{0}{1}D{2}N".format(element, self.domain_size,
                                                    self.domain_size + 1)
                condition_name = "{0}{1}D{2}N".format(
                    condition, self.domain_size, self.domain_size)

                KratosRANS.RansCalculationUtilities(
                ).CreateConnectivityPreservingModelPart(
                    self.fluid_model_part, model_part_name, element_name,
                    condition_name)
            else:
                Kratos.Logger.PrintInfo(
                    self.__class__.__name__, model_part_name +
                    " already exists. Using the existing model part.")

            self.turbulence_model_parts_list.append(
                self.model.GetModelPart(model_part_name))

    def AddVariables(self):
        msg = "Calling the base TurbulenceModelConfiguration class AddVariables method."
        msg += " Please override it in the derrived class."
        raise Exception(msg)

    def AddDofs(self):
        msg = "Calling the base TurbulenceModelConfiguration class AddDofs method."
        msg += " Please override it in the derrived class."
        raise Exception(msg)

    def GetTurbulenceSolvingProcess(self):
        msg = "Calling the base TurbulenceModelConfiguration class GetTurbulenceSolvingProcess method."
        msg += " Please override it in the derrived class to return a KratosMultiphysics.Process."
        raise Exception(msg)

    def Initialize(self):
        self.GetTurbulenceSolvingProcess().ExecuteInitialize()

    def Check(self):
        self.GetTurbulenceSolvingProcess().Check()

    def InitializeSolutionStep(self):
        self.GetTurbulenceSolvingProcess().ExecuteInitializeSolutionStep()

    def FinalizeSolutionStep(self):
        self.GetTurbulenceSolvingProcess().ExecuteFinalizeSolutionStep()