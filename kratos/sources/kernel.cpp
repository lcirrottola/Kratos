//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//

// System includes
#include <iostream>

// External includes

// Project includes
#include "includes/kernel.h"
#include "includes/kratos_version.h"
#include "input_output/logger.h"

namespace Kratos {
Kernel::Kernel(bool IsDistributedRun) : mpKratosCoreApplication(Kratos::make_shared<KratosApplication>(
                std::string("KratosMultiphysics"))), mIsDistributedRun(IsDistributedRun) {
    std::stringstream distributed_mode;
    if (IsDistributedRun)
    {
        distributed_mode << " -- MPI parallel run";
    }
    else {
        #ifdef _OPENMP
        distributed_mode << " -- OpenMP parallel run";
        #else
        distributed_mode << " -- Serial run";
        #endif
    }

    KRATOS_INFO("") << " |  /           |\n"
                    << " ' /   __| _` | __|  _ \\   __|\n"
                    << " . \\  |   (   | |   (   |\\__ \\\n"
                    << "_|\\_\\_|  \\__,_|\\__|\\___/ ____/\n"
                    << "           Multi-Physics " << KRATOS_VERSION << distributed_mode.str() << std::endl;

    if (!IsImported("KratosMultiphysics")) {
        this->ImportApplication(mpKratosCoreApplication);
    }
}

std::unordered_set<std::string> &Kernel::GetApplicationsList() {
  static std::unordered_set<std::string> application_list;
  return application_list;
}

bool Kernel::IsImported(std::string ApplicationName) const {
    if (GetApplicationsList().find(ApplicationName) !=
        GetApplicationsList().end())
        return true;
    else
        return false;
}

bool Kernel::IsDistributedRun() const {
    return mIsDistributedRun;
}

void Kernel::ImportApplication(KratosApplication::Pointer pNewApplication) {
    if (IsImported(pNewApplication->Name()))
        KRATOS_ERROR << "importing more than once the application : "
                     << pNewApplication->Name() << std::endl;

    pNewApplication->Register();
    Kernel::GetApplicationsList().insert(pNewApplication->Name());
}

std::string Kernel::Info() const { return "kernel"; }

void Kernel::PrintInfo(std::ostream& rOStream) const { rOStream << "kernel"; }

/// Print object's data.
void Kernel::PrintData(std::ostream& rOStream) const {
    rOStream << "Variables:" << std::endl;
    KratosComponents<VariableData>().PrintData(rOStream);
    rOStream << std::endl;
    rOStream << "Elements:" << std::endl;
    KratosComponents<Element>().PrintData(rOStream);
    rOStream << std::endl;
    rOStream << "Conditions:" << std::endl;
    KratosComponents<Condition>().PrintData(rOStream);

    rOStream << "Loaded applications:" << std::endl;

    auto& application_list = Kernel::GetApplicationsList();
    rOStream << "number of loaded applications = " << application_list.size()
             << std::endl;
    for (auto it = application_list.begin(); it != application_list.end(); ++it)
        rOStream << "  " << *it << std::endl;
}

std::string Kernel::BuildType() {
    return KRATOS_BUILD_TYPE;
}

std::string Kernel::Version() {
    return KRATOS_VERSION;
}

}
