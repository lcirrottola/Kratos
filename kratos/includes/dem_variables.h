// Kratos Multi-Physics
//
// Copyright (c) 2016, Pooyan Dadvand, Riccardo Rossi, CIMNE (International Center for Numerical Methods in Engineering)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 	-	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 	-	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
// 		in the documentation and/or other materials provided with the distribution.
// 	-	All advertising materials mentioning features or use of this software must display the following acknowledgement:
// 			This product includes Kratos Multi-Physics technology.
// 	-	Neither the name of the CIMNE nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED ANDON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THISSOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#if !defined(KRATOS_DEM_VARIABLES_H_INCLUDED )
#define  KRATOS_DEM_VARIABLES_H_INCLUDED



// System includes
#include <string>
#include <iostream>

// External includes


// Project includes
#include "includes/define.h"
#include "containers/variable.h"
#include "containers/variable_component.h"
#include "containers/vector_component_adaptor.h"
#include "includes/kratos_components.h"
#include "includes/ublas_interface.h"
#include "containers/array_1d.h"
#include "containers/weak_pointer_vector.h"
#include "containers/periodic_variables_container.h"

#undef  KRATOS_EXPORT_MACRO
#define KRATOS_EXPORT_MACRO KRATOS_API

//TODO: move to the Kratos DEM_application or eventually to the FluidDynamicsAsNeeded
namespace Kratos
{
     //for DEM Application:
    KRATOS_DEFINE_VARIABLE( double, PARTICLE_MASS )
    KRATOS_DEFINE_VARIABLE( double, RADIUS )
    KRATOS_DEFINE_VARIABLE( double, SEARCH_TOLERANCE )
    KRATOS_DEFINE_VARIABLE( double, AMPLIFIED_CONTINUUM_SEARCH_RADIUS_EXTENSION )
    KRATOS_DEFINE_VARIABLE( double, SEARCH_RADIUS )
    KRATOS_DEFINE_VARIABLE( double, DEM_DELTA_TIME )

    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( DAMP_FORCES )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( AUX_VEL )

    KRATOS_DEFINE_VARIABLE( Vector, NEIGHBOURS_IDS_DOUBLE )
    KRATOS_DEFINE_VARIABLE( Vector, PARTICLE_ROTATE_SPRING_FAILURE_TYPE )

    KRATOS_DEFINE_VARIABLE( vector<int>, OLD_NEIGHBOURS_IDS )
    KRATOS_DEFINE_VARIABLE( vector<int>, INI_NEIGHBOURS_IDS )
    KRATOS_DEFINE_VARIABLE( vector<int>, CONTINUUM_INI_NEIGHBOURS_IDS )
    KRATOS_DEFINE_VARIABLE( vector<int>, NEIGHBOURS_IDS )
    KRATOS_DEFINE_VARIABLE( vector<int>, PARTICLE_INITIAL_FAILURE_ID )
    KRATOS_DEFINE_VARIABLE( vector<int>, CONTINUUM_PARTICLE_INITIAL_FAILURE_ID )

    KRATOS_DEFINE_VARIABLE( int, FIXED_MESH_OPTION)
    KRATOS_DEFINE_VARIABLE( int, PARTICLE_MATERIAL )

    KRATOS_DEFINE_VARIABLE( std::string, ELEMENT_TYPE )

    typedef vector<array_1d<double,3> > VectorArray3Double;
    KRATOS_DEFINE_VARIABLE( VectorArray3Double, PARTICLE_ROTATE_SPRING_MOMENT )

    // Swimming DEM Application BEGINNING
    KRATOS_DEFINE_VARIABLE( int, COUPLING_TYPE)
    KRATOS_DEFINE_VARIABLE( int, NON_NEWTONIAN_OPTION )
    KRATOS_DEFINE_VARIABLE( int, MANUALLY_IMPOSED_DRAG_LAW_OPTION )
    KRATOS_DEFINE_VARIABLE( int, DRAG_MODIFIER_TYPE )
    KRATOS_DEFINE_VARIABLE( int, BUOYANCY_FORCE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, DRAG_FORCE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, VIRTUAL_MASS_FORCE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, LIFT_FORCE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, MAGNUS_FORCE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, HYDRO_TORQUE_TYPE )
    KRATOS_DEFINE_VARIABLE( int, FLUID_MODEL_TYPE )
    KRATOS_DEFINE_VARIABLE( int, DRAG_POROSITY_CORRECTION_TYPE )
    KRATOS_DEFINE_VARIABLE( double, POWER_LAW_TOLERANCE )
    KRATOS_DEFINE_VARIABLE( double, PARTICLE_SPHERICITY )
    KRATOS_DEFINE_VARIABLE( double, INIT_DRAG_FORCE )
    KRATOS_DEFINE_VARIABLE( double, DRAG_LAW_SLOPE )
    KRATOS_DEFINE_VARIABLE( double, SOLID_FRACTION )
    KRATOS_DEFINE_VARIABLE( double, SOLID_FRACTION_RATE )
    KRATOS_DEFINE_VARIABLE( double, FLUID_FRACTION )
    KRATOS_DEFINE_VARIABLE( double, FLUID_FRACTION_RATE )
    KRATOS_DEFINE_VARIABLE( double, PHASE_FRACTION )
    KRATOS_DEFINE_VARIABLE( double, PHASE_FRACTION_RATE )
    KRATOS_DEFINE_VARIABLE( double, SOLID_FRACTION_PROJECTED )
    KRATOS_DEFINE_VARIABLE( double, FLUID_FRACTION_PROJECTED )
    KRATOS_DEFINE_VARIABLE( double, FLUID_DENSITY_PROJECTED )
    KRATOS_DEFINE_VARIABLE( double, FLUID_VISCOSITY_PROJECTED )
    KRATOS_DEFINE_VARIABLE( double, REYNOLDS_NUMBER )
    KRATOS_DEFINE_VARIABLE( double, SHEAR_RATE_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( HYDRODYNAMIC_FORCE )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( HYDRODYNAMIC_MOMENT )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( FLUID_VEL_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( FLUID_ACCEL_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( FLUID_VORTICITY_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( HYDRODYNAMIC_REACTION )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( DRAG_REACTION )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( LIFT_FORCE )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( VIRTUAL_MASS_FORCE )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( BUOYANCY )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( PRESSURE_GRADIENT )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( PRESSURE_GRAD_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( SOLID_FRACTION_GRADIENT )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( SOLID_FRACTION_GRADIENT_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( FLUID_FRACTION_GRADIENT )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( FLUID_FRACTION_GRADIENT_PROJECTED )
    KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS( PHASE_FRACTION_GRADIENT )
    // Swimming DEM Application END

}  // namespace Kratos.

#undef  KRATOS_EXPORT_MACRO
#define KRATOS_EXPORT_MACRO KRATOS_NO_EXPORT

#endif // KRATOS_DEM_VARIABLES_H_INCLUDED  defined
