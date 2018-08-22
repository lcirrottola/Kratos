//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Aditya Ghantasala
//
//

#ifndef APPLY_PERIODIC_CONDITION_PROCESS_H
#define APPLY_PERIODIC_CONDITION_PROCESS_H

// System includes
#include <string>
#include <iostream>
#include <chrono>

// External includes

// Project includes
#include "includes/define.h"
#include "geometries/point.h"
#include "processes/process.h"
#include "utilities/math_utils.h"
#include "includes/kratos_parameters.h"
#include "utilities/binbased_fast_point_locator_on_conditions.h"



namespace Kratos
{

class ApplyPeriodicConditionProcess : public Process
{

  public:
    /// Pointer definition of ApplyPeriodicConditionProcess
    KRATOS_CLASS_POINTER_DEFINITION(ApplyPeriodicConditionProcess);

    typedef Dof<double> *DofPointerType;
    typedef Dof<double> DofType;

    typedef ModelPart::VariableComponentType VariableComponentType;
    typedef KratosComponents<Variable<array_1d<double, 3>>> VectorVariableType;
    typedef ProcessInfo ProcessInfoType;
    typedef ProcessInfo::Pointer ProcessInfoPointerType;
    typedef unsigned int IndexType;

    typedef ModelPart::DoubleVariableType VariableType;
    typedef ModelPart::NodeIterator NodeIterator;
    typedef Element ElementType;
    typedef Node<3> NodeType;
    typedef Matrix MatrixType;
    typedef Vector VectorType;
    typedef Geometry<NodeType> GeomertyType;

    ApplyPeriodicConditionProcess(ModelPart &model_part,
                                  Parameters rParameters) : Process(Flags()), mrMainModelPart(model_part), mParameters(rParameters)
    {
        Parameters default_parameters(R"(
                                            {
                                            "master_sub_model_part_name":"default_master",
                                            "slave_sub_model_part_name":"default_slave",
                                            "variable_names":[],
                                            "center":[0,0,0],
                                            "axis_of_rotation":[0.0,0.0,0.0],
                                            "angle":0.0,
                                            "flip":false,
                                            "dir_of_translation":[0.0,0.0,0.0],
                                            "magnitude":0.0
                                            }  )");

        // Initializing
        mParameters.RecursivelyValidateAndAssignDefaults(default_parameters);

        // Use the salve model part and newly created model part to form MPC constraints
        mSlaveSubModelPartName = mParameters["slave_sub_model_part_name"].GetString();
        mMasterSubModelPartName = mParameters["master_sub_model_part_name"].GetString();

        mCenterOfRotation.push_back(mParameters["center"][0].GetDouble());
        mCenterOfRotation.push_back(mParameters["center"][1].GetDouble());
        mCenterOfRotation.push_back(mParameters["center"][2].GetDouble());

        mAxisOfRotationVector.push_back(mParameters["axis_of_rotation"][0].GetDouble());
        mAxisOfRotationVector.push_back(mParameters["axis_of_rotation"][1].GetDouble());
        mAxisOfRotationVector.push_back(mParameters["axis_of_rotation"][2].GetDouble());

        mModulus = -1*mParameters["magnitude"].GetDouble(); // negative because we move the slave node not the master.
        mDirOfTranslation.push_back(mParameters["dir_of_translation"][0].GetDouble());
        mDirOfTranslation.push_back(mParameters["dir_of_translation"][1].GetDouble());
        mDirOfTranslation.push_back(mParameters["dir_of_translation"][2].GetDouble());

        if (mParameters["flip"].GetBool())
            mSign = -1;
        else
            mSign = 1;

        mTheta = -1*mParameters["angle"].GetDouble() * 2 * 3.1416 / 360.0; // negative because we move the slave node not the master.

        mTransformationMatrix.resize(4,4);

        if (mTheta == 0 && mModulus != 0)
            mType = "translation";
        else if (mTheta != 0.0 && mModulus == 0.0)
            mType = "rotation";
        else if (mTheta == 0.0 && mModulus == 0.0)
            KRATOS_THROW_ERROR(std::runtime_error, "Both angle of rotation and modulus of translation cannot be zero. Please check the input", "");

        CalculateTransformationMatrix();
        mIsInitialized = false;
    }
    ~ApplyPeriodicConditionProcess()
    {
    }

    void ExecuteInitializeSolutionStep() override
    {
        KRATOS_TRY;
        if (!mIsInitialized)
        {
            ProcessInfoPointerType info = mrMainModelPart.pGetProcessInfo();
            int probDim = info->GetValue(DOMAIN_SIZE);
            // Rotate the master so it goes to the slave
            if (probDim == 2)
                ApplyConstraintsForPeriodicConditions<2>();
            else if (probDim == 3)
                ApplyConstraintsForPeriodicConditions<3>();
            mIsInitialized = true;
        }
        std::cout<<"Initialization of Periodic conditions finished .. !"<<std::endl;
        // Once master and slave nodes are on the same surface we interpolate and appply constraints
        KRATOS_CATCH("");
    }

    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream <<"ApplyPeriodicConditionProcess Process "<<std::endl;
    }

  private:
    MatrixType mTransformationMatrix; // This can be either for rotating or for translating the master or slave
    ModelPart &mrMainModelPart;       // the MainModelPart to which the master-slave constraints are added.
    Parameters mParameters;          // parameters
    ModelPart mpRotatedMasterModelPart; // This is new modelpart
    std::string mSlaveSubModelPartName;
    std::string mMasterSubModelPartName;
    double mTheta;
    bool mIsInitialized;
    std::vector<double> mCenterOfRotation;
    std::vector<double> mAxisOfRotationVector;
    int mSign;
    std::string mType;
    double mModulus;
    std::vector<double> mDirOfTranslation;

    /**
     * @brief   The function to figure out how the master and slave model parts relate together and add master-slave constraints 
     *          to the rModelPart
     */
    template <int TDim>
    void ApplyConstraintsForPeriodicConditions()
    {
        ModelPart &slave_model_part = mrMainModelPart.GetSubModelPart(mParameters["slave_sub_model_part_name"].GetString());
        std::cout<<"Number of nodes in slave_model_part :: "<<slave_model_part.NumberOfNodes()<<std::endl;
        ModelPart &master_model_part = mrMainModelPart.GetSubModelPart(mParameters["master_sub_model_part_name"].GetString());
        int num_vars = mParameters["variable_names"].size();
        std::cout<<"Number of conditions in master_model_part :: "<<master_model_part.NumberOfConditions()<<std::endl;
        BinBasedFastPointLocatorOnConditions<TDim> bin_based_point_locator(master_model_part);
        bin_based_point_locator.UpdateSearchDatabase();

        mrMainModelPart.MasterSlaveConstraints().reserve(9*slave_model_part.NumberOfNodes());
        //BruteForcePointLocator brute_force_point_locator(master_model_part);

        // for bin based point locator
		VectorType shape_function_values;
		const int max_results = 10000;
		typename BinBasedFastPointLocatorOnConditions<TDim>::ResultContainerType results(max_results);
        typename BinBasedFastPointLocatorOnConditions<TDim>::ResultIteratorType result_begin = results.begin();

        // Record start time
        auto start_time_loop = std::chrono::high_resolution_clock::now();

        for (int j = 0; j < num_vars; j++)
        {
            const std::string var_name = mParameters["variable_names"][j].GetString();
            if (KratosComponents<VariableComponent<VectorComponentAdaptor<array_1d<double, 3>>>>::Has(var_name + "_X"))
            {   // Checking if the variable is a vector variable
                const int num_slave_nodes = slave_model_part.NumberOfNodes();
                ModelPart::NodeIterator it_node_begin = slave_model_part.NodesBegin();
                int num_slaves_found = 0;
                //#pragma omp parallel for schedule(static) //firstprivate(bin_based_point_locator)
                for(int i_node = 0; i_node<num_slave_nodes; ++i_node)
                {
                    Condition::Pointer p_host_elem;
                    ModelPart::NodeIterator it_node = it_node_begin;
                    std::advance(it_node, i_node);
                    // Transforming the slave point to the master modelpart
                    array_1d<double, 3 > transformed_node;
                    TransformNode(it_node->Coordinates(), transformed_node);

                    // Finding the host element for this node
				    bool is_found = bin_based_point_locator.FindPointOnMesh(transformed_node, shape_function_values, p_host_elem, result_begin, max_results);
                    //IndexType is_found = brute_force_point_locator.FindCondition(Node<3>(0,transformed_node), shape_function_values);
                    if(is_found)
                    {
                        ++num_slaves_found;
                        std::cout<<"Found :: "<<num_slaves_found<<" out of "<< num_slave_nodes<<std::endl;
                        ConstraintSlaveNodeWithElement<TDim>(*it_node, p_host_elem->GetGeometry() , shape_function_values, var_name);
                        //ConstraintSlaveNodeWithElement<TDim>(slave_node, master_model_part.Conditions()[is_found].GetGeometry() , shape_function_values, var_name);
                    }
                }
            }
        }
        // Record end time
        auto finish_time_loop = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time_loop = finish_time_loop - start_time_loop;
        std::cout << "Time taken for the loop is : " << elapsed_time_loop.count() << " s\n";
    }

    template <int TDim>
    void ConstraintSlaveNodeWithElement(NodeType& rSalveNode, GeomertyType& rHosteGeometry, VectorType& rWeights, const std::string& rVarName )
    {
        VariableComponentType r_var_x = KratosComponents<VariableComponentType>::Get(rVarName + std::string("_X"));
        VariableComponentType r_var_y = KratosComponents<VariableComponentType>::Get(rVarName + std::string("_Y"));
        VariableComponentType r_var_z = KratosComponents<VariableComponentType>::Get(rVarName + std::string("_Z"));

        IndexType master_index = 0;
        for (auto& master_node : rHosteGeometry)
        {
                int current_num_constraint = mrMainModelPart.NumberOfMasterSlaveConstraints();

                double master_weight = mSign * rWeights(master_index);
                double constant_x(0.0), constant_y(0.0), constant_z(0.0);

                constant_x = master_weight * mTransformationMatrix(0,3);
                constant_y = master_weight * mTransformationMatrix(1,3);
                constant_z = master_weight * mTransformationMatrix(2,3);
                // Record start time
                auto start_time_con = std::chrono::high_resolution_clock::now();
                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_x, rSalveNode, r_var_x, master_weight * mTransformationMatrix(0,0), constant_x);
                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_y, rSalveNode, r_var_x, master_weight * mTransformationMatrix(0,1), constant_x);
                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_z, rSalveNode, r_var_x, master_weight * mTransformationMatrix(0,2), constant_x);

                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_x, rSalveNode, r_var_y, master_weight * mTransformationMatrix(1,0), constant_y);
                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_y, rSalveNode, r_var_y, master_weight * mTransformationMatrix(1,1), constant_y);
                mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_z, rSalveNode, r_var_y, master_weight * mTransformationMatrix(1,2), constant_y);

                if (TDim == 3)
                {
                    mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_x, rSalveNode, r_var_z, master_weight * mTransformationMatrix(2,0), constant_z);
                    mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_y, rSalveNode, r_var_z, master_weight * mTransformationMatrix(2,1), constant_z);
                    mrMainModelPart.CreateNewMasterSlaveConstraint("LinearMasterSlaveConstraint", current_num_constraint++, master_node, r_var_z, rSalveNode, r_var_z, master_weight * mTransformationMatrix(2,2), constant_z);
                }
                        auto finish_time_con = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> elapsed_time_con = finish_time_con - start_time_con;
                        //std::cout << "Time taken for constraining slave : " << elapsed_time_con.count() << " s\n";

            master_index++;
        }
    }

    /**
     * @brief   Fuctions calculates the transformation matrix to account for the moving the two periodic condition modelparts together.
     */
    void CalculateTransformationMatrix()
    {
        if (mType == "translation")
            CalculateTranslationMatrix();
        else if (mType == "rotation")
            CalculateRoatationMatrix();
    }

    /**
     * @brief   Calculates the transformation matrix which translates the given vector alone mDirOfTranslation by mModulus
     */
    void CalculateTranslationMatrix()
    {
        mTransformationMatrix(0,0) = 1;
        mTransformationMatrix(0,1) = 0;
        mTransformationMatrix(0,2) = 0;
        mTransformationMatrix(0,3) = mModulus * mDirOfTranslation[0];
        mTransformationMatrix(1,0) = 0;
        mTransformationMatrix(1,1) = 1.0;
        mTransformationMatrix(1,2) = 0;
        mTransformationMatrix(1,3) = mModulus * mDirOfTranslation[1];
        mTransformationMatrix(2,0) = 0;
        mTransformationMatrix(2,1) = 0;
        mTransformationMatrix(2,2) = 1.0;
        mTransformationMatrix(2,3) = mModulus * mDirOfTranslation[2];
        mTransformationMatrix(3,3) = 1.0;
    }

    /**
     * @brief   Calculates the transformation matrix which rotates the given vector around mAxisOfRotationVector and mCenterOfRotation
     *          by mTheta
     */
    void CalculateRoatationMatrix()
    {
        std::vector<double> U(3); // normalized axis of rotation
        // normalizing the axis of roatation
        double norm = 0.0;
        for (unsigned int d = 0; d < 3; ++d)
            norm += mAxisOfRotationVector[d] * mAxisOfRotationVector[d];
        norm = sqrt(norm);
        for (unsigned int d = 0; d < 3; ++d)
            U[d] = mAxisOfRotationVector[d] / norm;

        // Constructing the transformation matrix
        double x1 = mCenterOfRotation[0];
        double y1 = mCenterOfRotation[1];
        double z1 = mCenterOfRotation[2];

        double a = U[0];
        double b = U[1];
        double c = U[2];
  
        double t2 = cos(mTheta);
        double t3 = sin(mTheta);
        double t4 = a * a;
        double t5 = b * b;
        double t6 = c * c;
        double t7 = a * b;
        double t8 = t5 + t6;
        double t9 = 1.0 / t8;
        if(isnan(t9))
            t9 = 0.0;
        double t10 = a * c;
        double t11 = b * t3;
        double t12 = a * t3 * t5;
        double t13 = a * t3 * t6;
        double t14 = b * c * t2;
        mTransformationMatrix(0,0) = t4 + t2 * t8;
        mTransformationMatrix(0,1) = t7 - c * t3 - a * b * t2;
        mTransformationMatrix(0,2) = t10 + t11 - a * c * t2;
        mTransformationMatrix(0,3) = x1 - t4 * x1 - a * b * y1 - a * c * z1 - b * t3 * z1 + c * t3 * y1 - t2 * t5 * x1 - t2 * t6 * x1 + a * b * t2 * y1 + a * c * t2 * z1;
        mTransformationMatrix(1,0) = t7 + c * t3 - a * b * t2;
        mTransformationMatrix(1,1) = t9 * (t2 * t6 + t5 * t8 + t2 * t4 * t5);
        mTransformationMatrix(1,2) = -t9 * (t12 + t13 + t14 - b * c * t8 - b * c * t2 * t4);
        mTransformationMatrix(1,3) = -t9 * (-t8 * y1 + t2 * t6 * y1 + t5 * t8 * y1 + a * b * t8 * x1 - b * c * t2 * z1 + b * c * t8 * z1 - a * t3 * t5 * z1 - a * t3 * t6 * z1 + c * t3 * t8 * x1 + t2 * t4 * t5 * y1 - a * b * t2 * t8 * x1 + b * c * t2 * t4 * z1);
        mTransformationMatrix(2,0) = t10 - t11 - a * c * t2;
        mTransformationMatrix(2,1) = t9 * (t12 + t13 - t14 + b * c * t8 + b * c * t2 * t4);
        mTransformationMatrix(2,2) = t9 * (t2 * t5 + t6 * t8 + t2 * t4 * t6);
        mTransformationMatrix(2,3) = -t9 * (-t8 * z1 + t2 * t5 * z1 + t6 * t8 * z1 + a * c * t8 * x1 - b * c * t2 * y1 + b * c * t8 * y1 + a * t3 * t5 * y1 + a * t3 * t6 * y1 - b * t3 * t8 * x1 + t2 * t4 * t6 * z1 - a * c * t2 * t8 * x1 + b * c * t2 * t4 * y1);
        mTransformationMatrix(3,3) = 1.0;
    }


    /*
     * @brief Rotates a given point(node_cords) in space around a given mAxisOfRotationVector by an angle thetha
     */
    void TransformNode(array_1d<double, 3 >& rCoordinates, array_1d<double, 3 >& rRotatedCoordinates)
    {
        std::vector<double> originalNode(4, 0.0f);
        std::vector<double> rotatedNode(4, 0.0f);

        originalNode[0] = rCoordinates(0); originalNode[1] = rCoordinates(1); originalNode[2] = rCoordinates(2); originalNode[3] = 1.0;
        // Multiplying the point to get the rotated point
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                rotatedNode[i] += mTransformationMatrix(i,j) * originalNode[j];
            }
        }

        rRotatedCoordinates(0) = rotatedNode[0]; rRotatedCoordinates(1) = rotatedNode[1]; rRotatedCoordinates(2) = rotatedNode[2];
    }

};

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  ApplyPeriodicConditionProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const ApplyPeriodicConditionProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}


}  // namespace Kratos.

#endif // KRATOS_APPLY_CONSTANT_VECTORVALUE_PROCESS_H_INCLUDED  defined