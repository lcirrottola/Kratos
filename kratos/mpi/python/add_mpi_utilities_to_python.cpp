//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main author:     Jordi Cotela
//

// System includes

// External includes
#include <pybind11/pybind11.h>

// Project includes
#include "add_mpi_utilities_to_python.h"
#include "mpi/utilities/model_part_communicator_utilities.h"
#include "mpi/utilities/parallel_fill_communicator.h"
#include "mpi/utilities/gather_modelpart_utility.h"
#include "mpi/utilities/mpi_normal_calculation_utilities.h"

namespace Kratos {
namespace Python {

void AddMPIUtilitiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    py::class_<ModelPartCommunicatorUtilities>(m,"ModelPartCommunicatorUtilities")
    .def_static("SetMPICommunicator",&ModelPartCommunicatorUtilities::SetMPICommunicator)
    ;

    py::class_<ParallelFillCommunicator >(m,"ParallelFillCommunicator")
    .def(py::init<ModelPart& >() )
    .def("Execute", &ParallelFillCommunicator::Execute )
    .def("PrintDebugInfo", &ParallelFillCommunicator::PrintDebugInfo )
    ;

    py::class_<GatherModelPartUtility>(m, "GatherModelPartUtility")
        .def(py::init<int, ModelPart&, int, ModelPart&>())
        .def("GatherOnMaster", &GatherModelPartUtility::GatherOnMaster<double>)
        .def("GatherOnMaster", &GatherModelPartUtility::GatherOnMaster<array_1d<double, 3>>)
        .def("ScatterFromMaster", &GatherModelPartUtility::ScatterFromMaster<double>)
        .def("ScatterFromMaster",
             &GatherModelPartUtility::ScatterFromMaster<array_1d<double, 3>>);

    py::class_<MPINormalCalculationUtils, MPINormalCalculationUtils::Pointer>(m,"MPINormalCalculationUtils")
    .def(py::init<>())
    .def("Check",&MPINormalCalculationUtils::Check)
    .def("OrientFaces",&MPINormalCalculationUtils::OrientFaces)
    .def("CalculateOnSimplex",&MPINormalCalculationUtils::CalculateOnSimplex)
    ;

}

} // namespace Python
} // namespace Kratos

