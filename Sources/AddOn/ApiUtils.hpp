#pragma once

#include "ACAPinc.h"

#include "Sight.hpp"
#include "Model.hpp"
#include "Model3D/Model3D.hpp"

void SetAPIElementType (API_Element& element, API_ElemTypeID elemTypeId);
GS::UniString GetElemTypeName (const API_Elem_Head& elemHead);

GSErrCode GetAPIModel (Modeler::SightPtr sight, ModelerAPI::Model* model);
GSErrCode GetAPIModel (Modeler::ConstModel3DPtr modelPtr, ModelerAPI::Model* model);
