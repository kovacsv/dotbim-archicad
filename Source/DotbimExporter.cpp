#include "DotbimExporter.hpp"
#include "ModelEnumerator.hpp"
#include "PropertyHandler.hpp"
#include "MatrixUtils.hpp"
#include "ApiUtils.hpp"

#include <ACAPinc.h>

#include <AttributeIndex.hpp>
#include <ModelElement.hpp>
#include <ModelMeshBody.hpp>
#include <Polygon.hpp>
#include <ConvexPolygon.hpp>
#include <ModelMaterial.hpp>

#include <rapidjson.h>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace std
{

template <>
struct hash<Color>
{
    size_t operator() (const Color& color) const noexcept
    {
        return color.r + 12289 * color.g + 24593 * color.b + 49157 * color.a;
    }
};

template <>
struct hash<ModelerAPI::BaseElemId>
{
    size_t operator() (const ModelerAPI::BaseElemId& baseElemId) const noexcept
    {
        return baseElemId.GenerateHashValue ();
    }
};

}

static const Color DefaultElemColor (200, 200, 200, 255);

template <typename T>
static rapidjson::Value CreateArrayValue (rapidjson::Document& document, const std::vector<T>& items)
{
    auto& allocator = document.GetAllocator ();
    rapidjson::Value array (rapidjson::kArrayType);
    for (const T& item : items) {
        array.PushBack (item, allocator);
    }
    return array;
}

static rapidjson::Value CreateStringValue (rapidjson::Document& document, const std::string& str)
{
    auto& allocator = document.GetAllocator ();
    rapidjson::Value strValue;
    strValue.SetString (str.c_str (), (rapidjson::SizeType) str.length (), allocator);
    return strValue;
}

static rapidjson::Value CreateColorValue (rapidjson::Document& document, const Color& color)
{
    auto& allocator = document.GetAllocator ();
    rapidjson::Value colorObj (rapidjson::kObjectType);
    colorObj.AddMember ("r", color.r, allocator);
    colorObj.AddMember ("g", color.g, allocator);
    colorObj.AddMember ("b", color.b, allocator);
    colorObj.AddMember ("a", color.a, allocator);
    return colorObj;
}

class JsonBuilderEnumerator : public TriangleEnumerator
{
public:
    JsonBuilderEnumerator (rapidjson::Document& document) :
        document (document),
        coordinatesArray (rapidjson::kArrayType),
        indicesArray (rapidjson::kArrayType),
        faceColorsArray (rapidjson::kArrayType)
    {

    }

    virtual void OnVertex (const ModelerAPI::Vertex& vertex) override
    {
        auto& allocator = document.GetAllocator ();
        coordinatesArray.PushBack (vertex.x, allocator);
        coordinatesArray.PushBack (vertex.y, allocator);
        coordinatesArray.PushBack (vertex.z, allocator);
    }

    virtual void OnTriangle (const Color& color, Int32 v1, Int32 v2, Int32 v3) override
    {
        auto& allocator = document.GetAllocator ();

        indicesArray.PushBack (v1, allocator);
        indicesArray.PushBack (v2, allocator);
        indicesArray.PushBack (v3, allocator);

        faceColorsArray.PushBack (color.r, allocator);
        faceColorsArray.PushBack (color.g, allocator);
        faceColorsArray.PushBack (color.b, allocator);
        faceColorsArray.PushBack (color.a, allocator);

        usedColors.insert (color);
    }

    rapidjson::Document& document;

    rapidjson::Value coordinatesArray;
    rapidjson::Value indicesArray;
    rapidjson::Value faceColorsArray;

    std::unordered_set<Color> usedColors;
};

using BaseElemIdToMeshIndex = std::unordered_map<ModelerAPI::BaseElemId, rapidjson::SizeType>;

static void ExportElement (
    const ModelEnumerator& enumerator,
    UIndex elementIndex,
    rapidjson::Document& document,
    rapidjson::Value& meshesArray,
    rapidjson::Value& elementsArray,
    BaseElemIdToMeshIndex& baseElemIdToMeshIndex)
{
    auto& allocator = document.GetAllocator ();

    rapidjson::Value coordinatesArray (rapidjson::kArrayType);
    rapidjson::Value indicesArray (rapidjson::kArrayType);
    rapidjson::Value faceColorsArray (rapidjson::kArrayType);

    JsonBuilderEnumerator jsonBuilder (document);
    enumerator.EnumerateElementGeometry (elementIndex, jsonBuilder);
    if (jsonBuilder.indicesArray.Empty ()) {
        return;
    }

    bool needToAddNewMesh = true;
    rapidjson::SizeType meshId = meshesArray.Size ();
    ModelerAPI::BaseElemId baseElemId;
    if (enumerator.GetElementBaseElementId (elementIndex, baseElemId)) {
        auto found = baseElemIdToMeshIndex.find (baseElemId);
        if (found != baseElemIdToMeshIndex.end ()) {
            needToAddNewMesh = false;
            meshId = found->second;
        } else {
            baseElemIdToMeshIndex.insert ({ baseElemId, meshId });
        }
    }

    if (needToAddNewMesh) {
        rapidjson::Value meshObject (rapidjson::kObjectType);
        meshObject.AddMember ("mesh_id", meshId, allocator);
        meshObject.AddMember ("coordinates", jsonBuilder.coordinatesArray, allocator);
        meshObject.AddMember ("indices", jsonBuilder.indicesArray, allocator);
        meshesArray.PushBack (meshObject, allocator);
    }

    rapidjson::Value elementObject (rapidjson::kObjectType);
    elementObject.AddMember ("mesh_id", meshId, allocator);

    Vector3D translation (0.0, 0.0, 0.0);
    Quaternion rotation (0.0, 0.0, 0.0, 1.0);
    ModelerAPI::Transformation transformation;
    if (enumerator.GetElementTransformation (elementIndex, transformation)) {
        TRANMAT tranmat;
        transformation.ToTRANMAT (&tranmat);
        if (!tranmat.IsIdentity ()) {
            DecomposeMatrix (tranmat.GetMatrix (), translation, rotation);
        }
    }

    rapidjson::Value vectorObject (rapidjson::kObjectType);
    vectorObject.AddMember ("x", translation.x, allocator);
    vectorObject.AddMember ("y", translation.y, allocator);
    vectorObject.AddMember ("z", translation.z, allocator);
    elementObject.AddMember ("vector", vectorObject, allocator);

    rapidjson::Value rotationObject (rapidjson::kObjectType);
    rotationObject.AddMember ("qx", rotation.qx, allocator);
    rotationObject.AddMember ("qy", rotation.qy, allocator);
    rotationObject.AddMember ("qz", rotation.qz, allocator);
    rotationObject.AddMember ("qw", rotation.qw, allocator);
    elementObject.AddMember ("rotation", rotationObject, allocator);

    if (jsonBuilder.usedColors.size () == 1) {
        const Color& elemColor = *jsonBuilder.usedColors.begin ();
        elementObject.AddMember ("color", CreateColorValue (document, elemColor), allocator);
    } else {
        elementObject.AddMember ("color", CreateColorValue (document, DefaultElemColor), allocator);
        elementObject.AddMember ("face_colors", jsonBuilder.faceColorsArray, allocator);
    }

    const GS::Guid& elementGuid = enumerator.GetElementGuid (elementIndex);
    std::string elementGuidStr = elementGuid.ToString ().ToCStr ();
    elementObject.AddMember ("guid", CreateStringValue (document, elementGuidStr), allocator);

    API_Elem_Head apiElemHead = {};
    apiElemHead.guid = GSGuid2APIGuid (elementGuid);
    ACAPI_Element_GetHeader (&apiElemHead);

    GS::UniString elemTypeName = GetElemTypeName (apiElemHead);
    std::string elementTypeNameStr (elemTypeName.ToCStr ().Get ());
    elementObject.AddMember ("type", CreateStringValue (document, elementTypeNameStr), allocator);

    rapidjson::Value infoObject (rapidjson::kObjectType);
    EnumerateElemProperties (elementGuid, [&](const std::string& name, const std::string& val) {
        infoObject.AddMember (CreateStringValue (document, name), CreateStringValue (document, val), allocator);
    });
    elementObject.AddMember ("info", infoObject, allocator);

    elementsArray.PushBack (elementObject, allocator);
}

std::string ExportDotbim (const ModelerAPI::Model& model)
{
    ModelEnumerator enumerator (model);

    rapidjson::Document document (rapidjson::kObjectType);
    auto& allocator = document.GetAllocator ();

    document.AddMember ("schema_version", "1.1.0", allocator);
    rapidjson::Value meshesArray (rapidjson::kArrayType);
    rapidjson::Value elementsArray (rapidjson::kArrayType);

    BaseElemIdToMeshIndex baseElemIdToMeshIndex;
    for (UIndex elementIndex = 0; elementIndex < enumerator.GetElementCount (); ++elementIndex) {
        ExportElement (enumerator, elementIndex, document, meshesArray, elementsArray, baseElemIdToMeshIndex);
    }

    document.AddMember ("meshes", meshesArray, allocator);
    document.AddMember ("elements", elementsArray, allocator);

    rapidjson::Value infoObject (rapidjson::kObjectType);
    EnumerateProjectProperties ([&](const std::string& name, const std::string& val) {
        infoObject.AddMember (CreateStringValue (document, name), CreateStringValue (document, val), allocator);
    });
    document.AddMember ("info", infoObject, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer (buffer);
    document.Accept (writer);

    return buffer.GetString ();
}

GSErrCode ExportDotbimFile (const ModelerAPI::Model& model, const IO::Location& location)
{
    IO::File output (location, IO::File::OnNotFound::Create);
    if (output.GetStatus () != NoError) {
        return Error;
    }

    output.Open (IO::File::OpenMode::WriteEmptyMode);
    if (output.GetStatus () != NoError) {
        return Error;
    }

    std::string dotbimContent = ExportDotbim (model);
    output.WriteBin (dotbimContent.c_str (), (USize) dotbimContent.length ());

    output.Close ();
    return NoError;
}

GSErrCode ExportDotbimFileFrom3DWindow (const IO::Location& location)
{
    API_WindowInfo windowInfo = {};
    if (ACAPI_Window_GetCurrentWindow (&windowInfo) != NoError) {
        return Error;
    }
    if (windowInfo.typeID != APIWind_3DModelID) {
        return Error;
    }

    ModelerAPI::Model model;
    void* currentSight = nullptr;
    if (ACAPI_Sight_GetCurrentWindowSight (&currentSight) != NoError) {
        return Error;
    }

    Modeler::SightPtr sightPtr ((Modeler::Sight*) currentSight);
    Modeler::ConstModel3DPtr modelPtr (sightPtr->GetMainModelPtr ());
    if (GetAPIModel (modelPtr, &model) != NoError) {
        return Error;
    }

    return ExportDotbimFile (model, location);
}
