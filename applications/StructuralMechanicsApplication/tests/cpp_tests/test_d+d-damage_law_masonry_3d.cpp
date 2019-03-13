// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                     license: structural_mechanics_application/license.txt
//
//  Main authors:    Philip Kalkbrenner
//                   Alejandro Cornejo
//

// System includes

// External includes

// Project includes
#include "includes/process_info.h"
#include "testing/testing.h"
#include "containers/model.h"

// Application includes

// Constitutive law
#include "custom_constitutive/d_plus_d_minus_damage_masonry_3d.h"
#include "includes/model_part.h"
#include "geometries/tetrahedra_3d_4.h"
#include "structural_mechanics_application_variables.h"

namespace Kratos
{
namespace Testing
{
// We test the d+ d- damage Constitutive Law (for masonry) 2D...
typedef Node<3> NodeType;

/**
* Check the correct calculation of the integrated stress with the CL's
*/

KRATOS_TEST_CASE_IN_SUITE(DPlusDMinusMasonry3DPureCompression, KratosStructuralMechanicsFastSuite)
{
    ConstitutiveLaw::Parameters cl_parameters;
    Properties material_properties;
    ProcessInfo process_info;
    Vector stress_vector, strain_vector;

    Model current_model;
    ModelPart& test_model_part = current_model.CreateModelPart("Main");

    NodeType::Pointer p_node_1 = test_model_part.CreateNewNode(1, 1.0, 0.0, 0.0);
    NodeType::Pointer p_node_2 = test_model_part.CreateNewNode(2, 0.5, 0.5, 0.0);
    NodeType::Pointer p_node_3 = test_model_part.CreateNewNode(3, 0.5, 0.1666666667, 0.5);
    NodeType::Pointer p_node_4 = test_model_part.CreateNewNode(4, 0.0, 0.0, 0.0);

    Tetrahedra3D4<NodeType> Geom = Tetrahedra3D4<NodeType>(p_node_1, p_node_2, p_node_3, p_node_4);

    stress_vector = ZeroVector(6);
    strain_vector = ZeroVector(6);
    strain_vector[0] = 0.0;
    strain_vector[1] = 0.0;
    strain_vector[2] = -0.014644;
    strain_vector[3] = 0.0;
    strain_vector[4] = 0.0;
    strain_vector[5] = 0.0;

    material_properties.SetValue(YOUNG_MODULUS, 3718.0e+6);
    material_properties.SetValue(POISSON_RATIO, 0.2);
    material_properties.SetValue(YIELD_STRESS_TENSION, 1.559e+6);
    material_properties.SetValue(FRACTURE_ENERGY_TENSION, 5.0e+3);
    material_properties.SetValue(DAMAGE_ONSET_STRESS_COMPRESSION, 10.0e+6);
    material_properties.SetValue(YIELD_STRESS_COMPRESSION, 17.99e+6);
    material_properties.SetValue(YIELD_STRAIN_COMPRESSION, 0.02);
    material_properties.SetValue(RESIDUAL_STRESS_COMPRESSION, 2.00e+6);
    material_properties.SetValue(BIAXIAL_COMPRESSION_MULTIPLIER, 1.20);
    material_properties.SetValue(FRACTURE_ENERGY_COMPRESSION, 6.0e+5);
    material_properties.SetValue(SHEAR_COMPRESSION_REDUCTOR, 0.16);
    material_properties.SetValue(BEZIER_CONTROLLER_C1, 0.65);
    material_properties.SetValue(BEZIER_CONTROLLER_C2,  0.55);
    material_properties.SetValue(BEZIER_CONTROLLER_C3, 1.5);
    material_properties.SetValue(TRIAXIAL_COMPRESSION_COEFFICIENT, 0.66);
    material_properties.SetValue(DELAY_TIME, 1.0);

    Flags cl_options;
    cl_options.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
    cl_options.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
    cl_options.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, false);

    //process_info.SetValue(DELTA_TIME, 0.1);

    cl_parameters.SetElementGeometry(Geom);
    cl_parameters.SetMaterialProperties(material_properties);
    cl_parameters.SetStrainVector(strain_vector);
    cl_parameters.SetStressVector(stress_vector);
    cl_parameters.SetProcessInfo(process_info);
    cl_parameters.SetOptions(cl_options);
    Matrix const_matrix(6,6);
    cl_parameters.SetConstitutiveMatrix(const_matrix);

    // Create the CL
    DamageDPlusDMinusMasonry3DLaw masonry3d_cl = DamageDPlusDMinusMasonry3DLaw();

    std::vector<double> masonry3d_res;
    masonry3d_res = {-2.87488e+06,  -2.87488e+06, -1.14995e+07, 0.0, 0.0, 0.0};

    Vector test_masonry3d_stress;
    masonry3d_cl.CalculateMaterialResponseCauchy(cl_parameters);
    test_masonry3d_stress = cl_parameters.GetStressVector();

    // Check the results
    for (int comp = 0; comp < 6; comp++) {
        KRATOS_CHECK_NEAR(test_masonry3d_stress[comp], masonry3d_res[comp], 0.0001e6);
    }
}

KRATOS_TEST_CASE_IN_SUITE(DPlusDMinusMasonry3DPureTension, KratosStructuralMechanicsFastSuite)
{
    ConstitutiveLaw::Parameters cl_parameters;
    Properties material_properties;
    ProcessInfo process_info;
    Vector stress_vector, strain_vector;

    Model current_model;
    ModelPart& test_model_part = current_model.CreateModelPart("Main");

    NodeType::Pointer p_node_1 = test_model_part.CreateNewNode(1, 1.0, 0.0, 0.0);
    NodeType::Pointer p_node_2 = test_model_part.CreateNewNode(2, 0.5, 0.5, 0.0);
    NodeType::Pointer p_node_3 = test_model_part.CreateNewNode(3, 0.5, 0.1666666667, 0.5);
    NodeType::Pointer p_node_4 = test_model_part.CreateNewNode(4, 0.0, 0.0, 0.0);

    Tetrahedra3D4<NodeType> Geom = Tetrahedra3D4<NodeType>(p_node_1, p_node_2, p_node_3, p_node_4);

    stress_vector = ZeroVector(6);
    strain_vector = ZeroVector(6);
    strain_vector[0] = 0.0;
    strain_vector[1] = 0.0;
    strain_vector[2] = 0.000728;
    strain_vector[3] = 0.0;
    strain_vector[4] = 0.0;
    strain_vector[5] = 0.0;

    material_properties.SetValue(YOUNG_MODULUS, 3718.0e+6);
    material_properties.SetValue(POISSON_RATIO, 0.2);
    material_properties.SetValue(YIELD_STRESS_TENSION, 1.559e+6);
    material_properties.SetValue(FRACTURE_ENERGY_TENSION, 5.0e+3);
    material_properties.SetValue(DAMAGE_ONSET_STRESS_COMPRESSION, 10.0e+6);
    material_properties.SetValue(YIELD_STRESS_COMPRESSION, 17.99e+6);
    material_properties.SetValue(YIELD_STRAIN_COMPRESSION, 0.02);
    material_properties.SetValue(RESIDUAL_STRESS_COMPRESSION, 2.00e+6);
    material_properties.SetValue(BIAXIAL_COMPRESSION_MULTIPLIER, 1.20);
    material_properties.SetValue(FRACTURE_ENERGY_COMPRESSION, 6.0e+5);
    material_properties.SetValue(SHEAR_COMPRESSION_REDUCTOR, 0.16);
    material_properties.SetValue(BEZIER_CONTROLLER_C1, 0.65);
    material_properties.SetValue(BEZIER_CONTROLLER_C2,  0.55);
    material_properties.SetValue(BEZIER_CONTROLLER_C3, 1.5);
    material_properties.SetValue(TRIAXIAL_COMPRESSION_COEFFICIENT, 0.66);
    material_properties.SetValue(DELAY_TIME, 1.0);

    Flags cl_options;
    cl_options.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
    cl_options.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
    cl_options.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, false);

    //process_info.SetValue(DELTA_TIME, 0.1);

    cl_parameters.SetElementGeometry(Geom);
    cl_parameters.SetMaterialProperties(material_properties);
    cl_parameters.SetStrainVector(strain_vector);
    cl_parameters.SetStressVector(stress_vector);
    cl_parameters.SetProcessInfo(process_info);
    cl_parameters.SetOptions(cl_options);
    Matrix const_matrix(6,6);
    cl_parameters.SetConstitutiveMatrix(const_matrix);

    // Create the CL
    DamageDPlusDMinusMasonry3DLaw masonry3d_cl = DamageDPlusDMinusMasonry3DLaw();

    std::vector<double> masonry3d_res;
    masonry3d_res = {371694.0,  371694.0, 1.48677e+06, 0.0, 0.0, 0.0};

    Vector test_masonry3d_stress;
    masonry3d_cl.CalculateMaterialResponseCauchy(cl_parameters);
    test_masonry3d_stress = cl_parameters.GetStressVector();

    // Check the results
    for (int comp = 0; comp < 6; comp++) {
        KRATOS_CHECK_NEAR(test_masonry3d_stress[comp], masonry3d_res[comp], 0.0001e6);
    }
}

KRATOS_TEST_CASE_IN_SUITE(DPlusDMinusMasonry3DMixedState, KratosStructuralMechanicsFastSuite)
{
    ConstitutiveLaw::Parameters cl_parameters;
    Properties material_properties;
    ProcessInfo process_info;
    Vector stress_vector, strain_vector;

    Model current_model;
    ModelPart& test_model_part = current_model.CreateModelPart("Main");

    NodeType::Pointer p_node_1 = test_model_part.CreateNewNode(1, 1.0, 0.0, 0.0);
    NodeType::Pointer p_node_2 = test_model_part.CreateNewNode(2, 0.5, 0.5, 0.0);
    NodeType::Pointer p_node_3 = test_model_part.CreateNewNode(3, 0.5, 0.1666666667, 0.5);
    NodeType::Pointer p_node_4 = test_model_part.CreateNewNode(4, 0.0, 0.0, 0.0);

    Tetrahedra3D4<NodeType> Geom = Tetrahedra3D4<NodeType>(p_node_1, p_node_2, p_node_3, p_node_4);

    stress_vector = ZeroVector(6);
    strain_vector = ZeroVector(6);
    strain_vector[0] = 0.0;
    strain_vector[1] = 0.0;
    strain_vector[2] = 0.0;
    strain_vector[3] = 0.0;
    strain_vector[4] = 0.0019998;
    strain_vector[5] = 0.0019998;


    material_properties.SetValue(YOUNG_MODULUS, 3718.0e+6);
    material_properties.SetValue(POISSON_RATIO, 0.2);
    material_properties.SetValue(YIELD_STRESS_TENSION, 1.559e+6);
    material_properties.SetValue(FRACTURE_ENERGY_TENSION, 5.0e+3);
    material_properties.SetValue(DAMAGE_ONSET_STRESS_COMPRESSION, 10.0e+6);
    material_properties.SetValue(YIELD_STRESS_COMPRESSION, 17.99e+6);
    material_properties.SetValue(YIELD_STRAIN_COMPRESSION, 0.02);
    material_properties.SetValue(RESIDUAL_STRESS_COMPRESSION, 2.00e+6);
    material_properties.SetValue(BIAXIAL_COMPRESSION_MULTIPLIER, 1.20);
    material_properties.SetValue(FRACTURE_ENERGY_COMPRESSION, 6.0e+5);
    material_properties.SetValue(SHEAR_COMPRESSION_REDUCTOR, 0.16);
    material_properties.SetValue(BEZIER_CONTROLLER_C1, 0.65);
    material_properties.SetValue(BEZIER_CONTROLLER_C2,  0.55);
    material_properties.SetValue(BEZIER_CONTROLLER_C3, 1.5);
    material_properties.SetValue(TRIAXIAL_COMPRESSION_COEFFICIENT, 0.66);
    material_properties.SetValue(DELAY_TIME, 1.0);

    Flags cl_options;
    cl_options.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
    cl_options.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
    cl_options.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, false);

    //process_info.SetValue(DELTA_TIME, 0.1);

    cl_parameters.SetElementGeometry(Geom);
    cl_parameters.SetMaterialProperties(material_properties);
    cl_parameters.SetStrainVector(strain_vector);
    cl_parameters.SetStressVector(stress_vector);
    cl_parameters.SetProcessInfo(process_info);
    cl_parameters.SetOptions(cl_options);
    Matrix const_matrix(6,6);
    cl_parameters.SetConstitutiveMatrix(const_matrix);

    // Create the CL
    DamageDPlusDMinusMasonry3DLaw masonry3d_cl = DamageDPlusDMinusMasonry3DLaw();

    std::vector<double> masonry3d_res;
    masonry3d_res = {-1.16725e+06, -1.79269e-26, -1.16725e+06, -1.44655e-10, 2.55053e+06, 3.71778e+06};

    Vector test_masonry3d_stress;
    masonry3d_cl.CalculateMaterialResponseCauchy(cl_parameters);
    test_masonry3d_stress = cl_parameters.GetStressVector();

    // Check the results
    for (int comp = 0; comp < 6; comp++) {
        KRATOS_CHECK_NEAR(test_masonry3d_stress[comp], masonry3d_res[comp], 0.0001e6);
    }
}
} // namespace Testing
} // namespace Kratos