from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import KratosMultiphysics
KratosMultiphysics.CheckForPreviousImport()

def GetDefaultInputParameters():
    
    default_settings = KratosMultiphysics.Parameters("""
        {
            "coupling_level_type"                    : 1,
            "time_averaging_type"                    : 0,
            "interaction_start_time"                 : 0.0,
            "pick_individual_forces_option"          : false,
            "include_faxen_terms_option"             : false,
            "include_faxen_terms_option_comment"     : "(relevant if the Maxey Riley equation is used)",
            "gradient_calculation_type"              : 1,
            "gradient_calculation_type_comment"      : "(Not calculated (0), volume-weighed average(1), Superconvergent recovery(2))",
            "laplacian_calculation_type"             : 2,
            "laplacian_calculation_type_comment"     : "(Not calculated (0), Finite element projection (1), Superconvergent recovery(2))",
            "buoyancy_force_type"                    : 2,
            "buoyancy_force_type_comment"            : "null buoyancy (0), compute buoyancy (1)  if drag_force_type is 2 buoyancy is always parallel to gravity",
            "drag_force_type"                        : 10,
            "drag_force_type_comment"                : " null drag (0), Stokes (1), Weatherford (2), Ganser (3), Ishii (4), Newtonian Regime (5)",
            "virtual_mass_force_type"                : 10,
            "virtual_mass_force_type_comment"        : "null virtual mass force (0)",
            "lift_force_type"                        : 0,
            "lift_force_type_comment"                        : "# null lift force (0), Saffman (1)",
            "magnus_force_type"                      : 0,
            "magnus_force_type_comment"              : " null magnus force (0), Rubinow and Keller (1), Oesterle and Bui Dihn (2)",
            "hydro_torque_type"                      : 0,
            "hydro_torque_type_comment"              : "null hydrodynamic torque (0), Dennis (1)",
            "drag_modifier_type"                     : 0,
            "viscosity_modification_type"            : 0,


            "coupling_weighing_type"                 : 2,
            "coupling_weighing_type_comment"         : "{fluid_to_DEM, DEM_to_fluid, fluid_fraction} = {lin, lin, imposed} (-1), {lin, const, const} (0), {lin, lin, const} (1), {lin, lin, lin} (2), averaging method (3)",
            "fluid_model_type"                       : 1,
            "fluid_model_type_comment"               : " untouched, velocity incremented by 1/fluid_fraction (0), modified mass conservation only (1)",
            "coupling_scheme_type"                   : "UpdatedFluid",
            "coupling_scheme_type_comment"           : " UpdatedFluid, UpdatedDEM",
            "print_particles_results_option"         : false,
            "add_each_hydro_force_option"            : true,
            "add_each_hydro_force_option_comment"    : " add each of the hydrodynamic forces (drag, lift and virtual mass)",
            "project_at_every_substep_option"        : true,
            "velocity_trap_option"                   : false,
            "inlet_option"                           : true,
            "manually_imposed_drag_law_option"       : false,
            "stationary_problem_option"              : false,
            "stationary_problem_option_comment"      : " stationary, stop calculating the fluid after it reaches the stationary state",
            "flow_in_porous_medium_option"           : false,
            "flow_in_porous_medium_option_comment"   : " the porosity is an imposed field",
            "flow_in_porous_DEM_medium_option"       : false,
            "flow_in_porous_DEM_medium_option_comment" : "the DEM part is kept static",
            "embedded_option"                        : false,
            "embedded_option_comment"                : "the embedded domain tools are to be used",
            "make_results_directories_option"        : true,
            "make_results_directories_option_comment": "results are written into a folder (../results) inside the problem folder",
            "body_force_on_fluid_option"             : true,
            "print_debug_info_option"                : false,
            "print_debug_info_option_comment"        : " print a summary of global physical measures",
            "print_particles_results_cycle"          : 1,
            "print_particles_results_cycle_comment"  : " number of 'ticks' per printing cycle",
            "debug_tool_cycle"                       : 10,
            "debug_tool_cycle_comment"                       : " number of 'ticks' per debug computations cycle",
            "similarity_transformation_type"         : 0,
            "similarity_transformation_type_comment" : " no transformation (0), Tsuji (1)",
            "dem_inlet_element_type"                 : "SphericSwimmingParticle3D",
            "dem_inlet_element_type_comment"         : " SphericParticle3D, SphericSwimmingParticle3D",
            "drag_modifier_type"                     : 2,
            "drag_modifier_type_comment"             : " Hayder (2), Chien (3) # problemtype option",
            "drag_porosity_correction_type"          : 0,
            "drag_porosity_correction_type_comment"  : " No correction (0), Richardson and Zaki (1)",
            "min_fluid_fraction"                     : 0.2,
            "initial_drag_force"                     : 0.0,
            "drag_law_slope"                         : 0.0,
            "power_law_tol"                          : 0.0,
            "model_over_real_diameter_factor"        : 1.0,
            "model_over_real_diameter_factor_comment": " not active if similarity_transformation_type = 0",
            "max_pressure_variation_rate_tol"        : 1e-3,
            "max_pressure_variation_rate_tol_comment": " for stationary problems, criterion to stop the fluid calculations",
            "time_steps_per_stationarity_step"       : 15,
            "time_steps_per_stationarity_step_comment": " number of fluid time steps between consecutive assessment of stationarity steps",
            "meso_scale_length"                      : 0.2,
            "meso_scale_length_comment"              : " the radius of the support of the averaging function for homogenization (<=0 for automatic calculation)",
            "shape_factor"                           : 0.5,
            "non_newtonian_option"                   : false,
            "yield_stress"                           : 0.0,
            "regularization_coefficient"             : 0.0,
            "power_law_k"                            : 0.0,
            "power_law_n"                            : 0.0,

            "GraphExportFreq"                  : 1e-3,
            "VelTrapGraphExportFreq"           : 1e-3,
            "OutputTimeStep"                   : 0.5,
            "PostDisplacement"                 : true,
            "PostVelocity"                     : true,
            "PostElasticForces"                : false,
            "PostContactForces"                : false,
            "PostRigidElementForces"           : false,
            "PostTangentialElasticForces"      : false,
            "PostTotalForces"                  : false,
            "PostShearStress"                  : false,
            "PostNonDimensionalVolumeWear"     : false,
            "PostNodalArea"                    : false,
            "PostRHS"                          : false,
            "PostDampForces"                   : false,
            "PostAppliedForces"                : false,
            "PostRadius"                       : true,
            "PostGroupId"                      : false,
            "PostExportId"                     : false,
            "PostAngularVelocity"              : false,
            "PostParticleMoment"               : false,
            "PostEulerAngles"                  : false,
            "PostBoundingBox"                  : false,
            "PostPressure"                     : false,
            "PostFluidPressure"                          : false,
            "print_REYNOLDS_NUMBER_option"               : false,
            "print_PRESSURE_GRAD_PROJECTED_option"       : false,
            "print_FLUID_VEL_PROJECTED_option"           : true,
            "print_FLUID_ACCEL_PROJECTED_option"         : true,
            "print_BUOYANCY_option"                      : true,
            "print_DRAG_FORCE_option"                    : true,
            "print_VIRTUAL_MASS_FORCE_option"            : true,
            "print_BASSET_FORCE_option"                  : true,
            "print_LIFT_FORCE_option"                    : true,
            "print_FLUID_VEL_PROJECTED_RATE_option"      : true,
            "print_FLUID_VISCOSITY_PROJECTED_option"     : false,
            "print_FLUID_FRACTION_PROJECTED_option"      : false,
            "print_FLUID_VEL_LAPL_PROJECTED_option"      : false,
            "print_FLUID_VEL_LAPL_RATE_PROJECTED_option" : false,
            "print_HYDRODYNAMIC_FORCE_option"            : true,
            "print_HYDRODYNAMIC_MOMENT_option"           : false,
            "print_MESH_VELOCITY1_option"                : false,
            "print_BODY_FORCE_option"                    : false,
            "print_FLUID_FRACTION_option"                : false,
            "print_FLUID_FRACTION_GRADIENT_option"       : false,
            "print_HYDRODYNAMIC_REACTION_option"         : false,
            "print_PRESSURE_option"                      : false,
            "print_PRESSURE_GRADIENT_option"             : false,
            "print_DISPERSE_FRACTION_option"             : false,
            "print_MEAN_HYDRODYNAMIC_REACTION_option"    : false,
            "print_VELOCITY_LAPLACIAN_option"            : false,
            "print_VELOCITY_LAPLACIAN_RATE_option"       : false,
            
            "PredefinedSkinOption"             : false,
            "MeanRadius"                       : 0.0001
            }""")
            
    return default_settings