#include "DotbimImporter.hpp"
#include "MatrixUtils.hpp"

#include "ApiUtils.hpp"

#include <ACAPinc.h>

#include <Color.hpp>
#include <File.hpp>
#include <TRANMAT.h>
#include <StringConversion.hpp>

#include <rapidjson.h>
#include <document.h>

#include <vector>

static API_AttributeIndex CreateMaterial (const Gfx::Color& color, const GS::UniString& nameTemplate)
{
    API_Attribute material = {};

    material.header.typeID = API_MaterialID;
    GS::UniString colorString = "(" +
        GS::ValueToString (color.GetRed ()) + ", " +
        GS::ValueToString (color.GetGreen ()) + ", " +
        GS::ValueToString (color.GetBlue ()) + ", " +
        GS::ValueToString (color.GetAlpha ()) +
        ")";
    GS::UniString materialName = GS::UniString::Printf (nameTemplate, colorString.ToPrintf ());
    material.header.uniStringNamePtr = &materialName;

    if (ACAPI_Attribute_Get (&material) == NoError) {
        delete material.material.texture.fileLoc; // :(
        return material.header.index;
    }

    material.material.surfaceRGB = {
        color.GetRed () / 255.0,
        color.GetGreen () / 255.0,
        color.GetBlue () / 255.0
    };
    material.material.transpPc = (short) (100.0 - color.GetAlpha () / 255.0 * 100.0);

    // default values stolen from "Paint" materials
    material.material.specularRGB = { 1.0, 1.0, 1.0 };
    material.material.ambientPc = 93;
    material.material.diffusePc = 41;
    material.material.shine = 72;
    material.material.specularPc = 10;

    if (ACAPI_Attribute_Create (&material, nullptr) == NoError) {
        return material.header.index;
    }

    return APIInvalidAttributeIndex;
}

static GSErrCode ImportDotbimElement (
    const rapidjson::Value& meshes,
    const GS::HashTable<Int32, rapidjson::SizeType>& meshIdToIndex,
    const rapidjson::Value& element,
    GS::HashTable<Gfx::Color, API_AttributeIndex>& colorToMaterialIndex,
    const GS::UniString& materialNameTemplate)
{
    Int32 meshId = element["mesh_id"].GetInt ();
    if (!meshIdToIndex.ContainsKey (meshId)) {
        return Error;
    }

    rapidjson::SizeType meshIndex = meshIdToIndex[meshId];
    const rapidjson::Value& mesh = meshes[meshIndex];

    GSErrCode err = NoError;
    void* bodyData = nullptr;
    err = ACAPI_Body_Create (nullptr, nullptr, &bodyData);
    if (err != NoError) {
        return err;
    }

    std::vector<UInt32> vertexIndices;
    const rapidjson::Value& coordinates = mesh["coordinates"];
    for (rapidjson::SizeType i = 0; i < coordinates.Size (); i += 3) {
        API_Coord3D position = {
            coordinates[i + 0].GetDouble (),
            coordinates[i + 1].GetDouble (),
            coordinates[i + 2].GetDouble ()
        };
        UInt32 vertexIndex = 0;
        ACAPI_Body_AddVertex (bodyData, position, vertexIndex);
        vertexIndices.push_back (vertexIndex);
    }

    bool hasFaceColors = element.HasMember ("face_colors");
    const rapidjson::Value& defaultColorObj = element["color"];
    Gfx::Color defaultColor (
        (UChar) defaultColorObj["r"].GetInt (),
        (UChar) defaultColorObj["g"].GetInt (),
        (UChar) defaultColorObj["b"].GetInt (),
        (UChar) defaultColorObj["a"].GetInt ()
    );

    const rapidjson::Value& indices = mesh["indices"];
    for (rapidjson::SizeType i = 0; i < indices.Size (); i += 3) {
        UInt32 v0 = vertexIndices[indices[i + 0].GetInt ()];
        UInt32 v1 = vertexIndices[indices[i + 1].GetInt ()];
        UInt32 v2 = vertexIndices[indices[i + 2].GetInt ()];

        Int32 e0 = 0;
        Int32 e1 = 0;
        Int32 e2 = 0;
        ACAPI_Body_AddEdge (bodyData, v0, v1, e0);
        ACAPI_Body_AddEdge (bodyData, v1, v2, e1);
        ACAPI_Body_AddEdge (bodyData, v2, v0, e2);

        Gfx::Color polygonColor = defaultColor;
        if (hasFaceColors) {
            rapidjson::SizeType colorIndex = (i / 3) * 4;
            const rapidjson::Value& faceColors = element["face_colors"];
            polygonColor.SetRed ((UChar) faceColors[colorIndex + 0].GetInt ());
            polygonColor.SetGreen ((UChar) faceColors[colorIndex + 1].GetInt ());
            polygonColor.SetBlue ((UChar) faceColors[colorIndex + 2].GetInt ());
            polygonColor.SetAlpha ((UChar) faceColors[colorIndex + 3].GetInt ());
        }

#ifdef ServerMainVers_2700
        API_OverriddenAttribute material;
        if (colorToMaterialIndex.ContainsKey (polygonColor)) {
            material = colorToMaterialIndex[polygonColor];
        } else {
            material = CreateMaterial (polygonColor, materialNameTemplate);
            colorToMaterialIndex.Add (polygonColor, material.value);
        }
#else
        API_OverriddenAttribute material;
        material.overridden = true;

        if (colorToMaterialIndex.ContainsKey (polygonColor)) {
            material.attributeIndex = colorToMaterialIndex[polygonColor];
        } else {
            material.attributeIndex = CreateMaterial (polygonColor, materialNameTemplate);
            colorToMaterialIndex.Add (polygonColor, material.attributeIndex);
        }
#endif

        UInt32 triangleIndex = 0;
        ACAPI_Body_AddPolygon (bodyData, { e0, e1, e2 }, 0, material, triangleIndex);
    }

    API_ElementMemo bodyMemo = {};
    ACAPI_Body_Finish (bodyData, &bodyMemo.morphBody, &bodyMemo.morphMaterialMapTable);
    ACAPI_Body_Dispose (&bodyData);

    API_Element morphElement = {};
    SetAPIElementType (morphElement, API_MorphID);
    err = ACAPI_Element_GetDefaults (&morphElement, nullptr);
    if (err != NoError) {
        return err;
    }

    const rapidjson::Value& vectorObj = element["vector"];
    Geometry::Vector3D translation (
        vectorObj["x"].GetDouble (),
        vectorObj["y"].GetDouble (),
        vectorObj["z"].GetDouble ()
    );

    const rapidjson::Value& rotationObj = element["rotation"];
    Quaternion rotation (
        rotationObj["qx"].GetDouble (),
        rotationObj["qy"].GetDouble (),
        rotationObj["qz"].GetDouble (),
        rotationObj["qw"].GetDouble ()
    );

    Geometry::Matrix34 matrix = ComposeMatrix (translation, rotation);
    morphElement.morph.tranmat.tmx[0] = matrix.Get (0, 0);
    morphElement.morph.tranmat.tmx[1] = matrix.Get (0, 1);
    morphElement.morph.tranmat.tmx[2] = matrix.Get (0, 2);
    morphElement.morph.tranmat.tmx[3] = matrix.Get (0, 3);
    morphElement.morph.tranmat.tmx[4] = matrix.Get (1, 0);
    morphElement.morph.tranmat.tmx[5] = matrix.Get (1, 1);
    morphElement.morph.tranmat.tmx[6] = matrix.Get (1, 2);
    morphElement.morph.tranmat.tmx[7] = matrix.Get (1, 3);
    morphElement.morph.tranmat.tmx[8] = matrix.Get (2, 0);
    morphElement.morph.tranmat.tmx[9] = matrix.Get (2, 1);
    morphElement.morph.tranmat.tmx[10] = matrix.Get (2, 2);
    morphElement.morph.tranmat.tmx[11] = matrix.Get (2, 3);

    // seems like it doesn't have any effect
    morphElement.morph.edgeType = APIMorphEdgeType_HardHiddenEdge;
    err = ACAPI_Element_Create (&morphElement, &bodyMemo);
    if (err != NoError) {
        return err;
    }

    ACAPI_DisposeElemMemoHdls (&bodyMemo);
    return NoError;
}

static GSErrCode ImportDotbimContent (const char* fileContent, const GS::UniString& materialNameTemplate)
{
    rapidjson::Document document;
    document.Parse (fileContent);

    if (document.HasParseError ()) {
        return Error;
    }

    if (!document.HasMember ("meshes") || !document.HasMember ("elements")) {
        return Error;
    }

    GS::HashTable<Int32, rapidjson::SizeType> meshIdToIndex;
    rapidjson::Value meshes = document["meshes"].GetArray ();
    for (rapidjson::SizeType meshIndex = 0; meshIndex < meshes.Size (); ++meshIndex) {
        const rapidjson::Value& mesh = meshes[meshIndex];
        meshIdToIndex.Add (mesh["mesh_id"].GetInt (), meshIndex);
    }

    rapidjson::Value elements = document["elements"].GetArray ();
    GS::HashTable<Gfx::Color, API_AttributeIndex> colorToMaterialIndex;
    for (rapidjson::SizeType elementIndex = 0; elementIndex < elements.Size (); ++elementIndex) {
        const rapidjson::Value& element = elements[elementIndex];
        ImportDotbimElement (meshes, meshIdToIndex, element, colorToMaterialIndex, materialNameTemplate);
    }

    return NoError;
}

static bool HasRightToCreateMaterials ()
{
    API_TWAccessRights accessRight = APIMaterialsCreate;
    bool hasRight = false;
#ifdef ServerMainVers_2700
    GSErrCode err = ACAPI_Teamwork_GetTWAccessRight (accessRight, &hasRight);
#else
    GSErrCode err = ACAPI_Environment (APIEnv_GetTWAccessRightID, &accessRight, &hasRight);
#endif
    if (err != NoError) {
        return false;
}
    return hasRight;
}

GSErrCode ImportDotbim (const IO::Location& fileLocation, const GS::UniString& materialNameTemplate)
{
    if (!HasRightToCreateMaterials ()) {
        return false;
    }

    IO::File file (fileLocation);
    if (file.GetStatus () != NoError) {
        return Error;
    }

    if (file.Open (IO::File::OpenMode::ReadMode) != NoError) {
        return Error;
    }

    UInt64 fileSize = 0;
    if (file.GetDataLength (&fileSize) != NoError) {
        return Error;
    }

    GSErrCode result = NoError;
    char* fileContent = new char[fileSize + 1];
    if (file.ReadBin (fileContent, (USize) fileSize) == NoError) {
        fileContent[fileSize] = 0;
        try {
            result = ImportDotbimContent (fileContent, materialNameTemplate);
        } catch (...) {
            result = Error;
        }
    }
    delete[] fileContent;
    file.Close ();

    return result;
}
