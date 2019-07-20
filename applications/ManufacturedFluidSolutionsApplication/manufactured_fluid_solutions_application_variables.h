//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Miguel Maso Sotomayor
//

#ifndef KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION_VARIABLES_H_INCLUDED
#define KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/variables.h"
#include "includes/kratos_parameters.h"
#include "includes/kratos_application.h"

namespace Kratos
{
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, EXACT_VELOCITY )
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, VELOCITY_RELATIVE_ERROR )
KRATOS_DEFINE_APPLICATION_VARIABLE( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, double, EXACT_PRESSURE )
KRATOS_DEFINE_APPLICATION_VARIABLE( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, double, PRESSURE_RELATIVE_ERROR )
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, EXACT_MATERIAL_ACCELERATION )
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION, MATERIAL_ACCELERATION_ERROR )
}

#endif	/* KRATOS_MANUFACTURED_FLUID_SOLUTIONS_APPLICATION_VARIABLES_H_INCLUDED */