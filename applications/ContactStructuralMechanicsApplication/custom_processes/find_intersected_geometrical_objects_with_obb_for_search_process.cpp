// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:             BSD License
//                                       license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes

// External includes

// Project includes
#include "custom_processes/find_intersected_geometrical_objects_with_obb_for_search_process.h"

namespace Kratos
{
FindIntersectedGeometricalObjectsWithOBBForSearchProcess::FindIntersectedGeometricalObjectsWithOBBForSearchProcess(
    ModelPart& rPart1,
    ModelPart& rPart2,
    const double BoundingBoxFactor,
    const bool DebugOBB,
    const OBBHasIntersectionType IntersectionType
    ) : BaseType(rPart1, rPart2, BoundingBoxFactor, DebugOBB, IntersectionType)
{
}

/***********************************************************************************/
/***********************************************************************************/

FindIntersectedGeometricalObjectsWithOBBForSearchProcess::FindIntersectedGeometricalObjectsWithOBBForSearchProcess(
    Model& rModel,
    Parameters ThisParameters
    ) : BaseType(rModel.GetModelPart(ThisParameters["first_model_part_name"].GetString()),
        rModel.GetModelPart(ThisParameters["second_model_part_name"].GetString()))
{
    const Parameters default_parameters = GetDefaultParameters();
    ThisParameters.RecursivelyValidateAndAssignDefaults(default_parameters);
    BaseProcessType::mThisParameters = ThisParameters;

    // Checking that the names of the model parts are not empty (this is supposed to be already declared)
    const std::string& r_first_model_part_name = BaseProcessType::mThisParameters["first_model_part_name"].GetString();
    const std::string& r_second_model_part_name = BaseProcessType::mThisParameters["second_model_part_name"].GetString();

    KRATOS_ERROR_IF(r_first_model_part_name == "") << "first_model_part_name must be defined on parameters" << std::endl;
    KRATOS_ERROR_IF(r_second_model_part_name == "") << "second_model_part_name must be defined on parameters" << std::endl;

    // Setting the bounding box factor
    mBoundingBoxFactor = BaseProcessType::mThisParameters["bounding_box_factor"].GetDouble();

    // In case we consider the bounding box we set the BOUNDARY flag
    if (mBoundingBoxFactor > 0.0)
        this->Set(BOUNDARY, true);
    else
        this->Set(BOUNDARY, false);

    // If we debug OBB
    mDebugOBB = BaseProcessType::mThisParameters["debug_obb"].GetBool();

    // The intersection type
    mIntersectionType = BaseType::ConvertInter(BaseProcessType::mThisParameters["OBB_intersection_type"].GetString());

    // We create new properties for debugging
    if (BaseType::mDebugOBB) {
        this->GetModelPart1().CreateNewProperties(1001);
        this->GetModelPart1().CreateSubModelPart(this->GetModelPart1().Name() + "_AUXILIAR_DEBUG_OBB");
        this->GetModelPart2().CreateNewProperties(1002);
        this->GetModelPart2().CreateSubModelPart(this->GetModelPart2().Name() + "_AUXILIAR_DEBUG_OBB");
    }

    mLowerBBCoefficient = ThisParameters["lower_bounding_box_coefficient"].GetDouble();
    mHigherBBCoefficient = ThisParameters["higher_bounding_box_coefficient"].GetDouble();
}

/***********************************************************************************/
/***********************************************************************************/

void FindIntersectedGeometricalObjectsWithOBBForSearchProcess::SetOctreeBoundingBox()
{
    // Getting first iterators
    const auto it_node_begin_1 = BaseType::mrModelPart1.NodesBegin();
    const auto it_node_begin_2 = BaseType::mrModelPart2.NodesBegin();

    // Setting initial guess
    PointType low(it_node_begin_1->Coordinates());
    PointType high(it_node_begin_1->Coordinates());

    // Loop over all nodes in first modelpart
    for (IndexType i_node = 0 ; i_node < BaseType::mrModelPart1.NumberOfNodes(); ++i_node) {
        auto it_node = it_node_begin_1 + i_node;
        const array_1d<double,3>& r_coordinates = it_node->Coordinates();
        for (IndexType i = 0; i < 3; i++) {
            low[i] = r_coordinates[i] < low[i] ? r_coordinates[i] : low[i];
            high[i] = r_coordinates[i] > high[i] ? r_coordinates[i] : high[i];
        }
    }

    // Loop over all skin nodes
    for (IndexType i_node = 0 ; i_node < BaseType::mrModelPart2.NumberOfNodes(); ++i_node) {
        auto it_node = it_node_begin_2 + i_node;
        const array_1d<double,3>& r_coordinates = it_node->Coordinates();
        for (IndexType i = 0; i < 3; i++) {
            low[i] = r_coordinates[i] < low[i] ? r_coordinates[i] : low[i];
            high[i] = r_coordinates[i] > high[i] ? r_coordinates[i] : high[i];
        }
    }

    // Slightly increase the bounding box size to avoid problems with geometries in the borders
    // Note that std::numeric_limits<double>::double() is added for the 2D cases. Otherwise, the
    // third component will be 0, breaking the octree behaviour.
    for(IndexType i = 0 ; i < 3; i++) {
        low[i] -= std::abs(high[i] - low[i])*1e-3 + std::numeric_limits<double>::epsilon();
        high[i] += std::abs(high[i] - low[i])*1e-3 + std::numeric_limits<double>::epsilon();
    }

    // In case we consider the bounding box we extend box
    if (mBoundingBoxFactor > 0.0) {
        const std::size_t dimension = BaseType::mrModelPart1.GetProcessInfo()[DOMAIN_SIZE];
        for(IndexType i = 0 ; i < dimension; i++) {
            if (std::abs(high[i] - low[i]) < mBoundingBoxFactor) {
                low[i] -= mLowerBBCoefficient * mBoundingBoxFactor;
                high[i] += mHigherBBCoefficient * mBoundingBoxFactor;
            }
        }
    }

    // TODO: Octree needs refactoring to work with BoundingBox. Pooyan.
#ifdef KRATOS_USE_AMATRIX   // This macro definition is for the migration period and to be removed afterward please do not use it
    BaseType::mOctree.SetBoundingBox(low.data(), high.data());
#else
    BaseType::mOctree.SetBoundingBox(low.data().data(), high.data().data());
#endif // ifdef KRATOS_USE_AMATRIX
}

/***********************************************************************************/
/***********************************************************************************/

void FindIntersectedGeometricalObjectsWithOBBForSearchProcess::MarkIfIntersected(
    Condition& rCondition1,
    OtreeCellVectorType& rLeaves
    )
{
    if (rCondition1.Is(SLAVE)) {
        // We clear previously assign flag
        rCondition1.Reset(SELECTED);

        // The difference with the normal one is that we assign the flag to all "paired" conditions
        for (auto p_leaf : rLeaves) {
            auto& r_leaf = *(p_leaf->pGetObjects());
            // We clear previously assign flags
            for (auto p_condition_2 : r_leaf) {
                if (p_condition_2->IsNot(VISITED)) {
                    p_condition_2->Reset(SELECTED);
                }
            }

            // We iterate again and check intersection
            for (auto p_condition_2 : r_leaf) {
                if (p_condition_2->IsNot(VISITED)) {
                    if (HasIntersection(rCondition1.GetGeometry(), p_condition_2->GetGeometry())) {
                        rCondition1.Set(SELECTED);
                        p_condition_2->Set(SELECTED);
                    }
                    p_condition_2->Set(VISITED);
                }
            }
        }
        // Reset VISITED flag
        for (auto p_leaf : rLeaves) {
            auto& r_leaf = *(p_leaf->pGetObjects());
            for (auto p_condition_2 : r_leaf) {
                p_condition_2->Reset(VISITED);
            }
        }
    }
}

/***********************************************************************************/
/***********************************************************************************/

Parameters FindIntersectedGeometricalObjectsWithOBBForSearchProcess::GetDefaultParameters()
{
    Parameters default_parameters = Parameters(R"(
    {
        "first_model_part_name"           : "",
        "second_model_part_name"          : "",
        "bounding_box_factor"             : -1.0,
        "debug_obb"                       : false,
        "OBB_intersection_type"           : "SeparatingAxisTheorem",
        "lower_bounding_box_coefficient"  : 0.0,
        "higher_bounding_box_coefficient" : 1.0
    })" );

    return default_parameters;
}

}  // namespace Kratos.
