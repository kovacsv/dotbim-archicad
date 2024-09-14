#include "DotbimCommands.hpp"

#include "ApiUtils.hpp"
#include "DotbimExporter.hpp"
#include "DotbimImporter.hpp"
#include "ResourceIds.hpp"

#include "RS.hpp"

GS::ObjectState CreateErrorResponse (GSErrCode errorCode, const GS::UniString& errorMessage)
{
    GS::ObjectState error;
    error.Add ("code", errorCode);
    error.Add ("message", errorMessage.ToCStr ().Get ());
    return GS::ObjectState ("error", error);
}

GS::ObjectState CreateFailedExecutionResult (GSErrCode errorCode, const GS::UniString& errorMessage)
{
    GS::ObjectState error = CreateErrorResponse (errorCode, errorMessage);
    error.Add ("success", false);
    return error;
}

GS::ObjectState CreateSuccessfulExecutionResult ()
{
    return GS::ObjectState ("success", true);
}

CommandBase::CommandBase () :
    API_AddOnCommand ()
{
}

GS::String CommandBase::GetNamespace () const
{
    return "Bimdots";
}

GS::Optional<GS::UniString> CommandBase::GetSchemaDefinitions () const
{
    return GS::NoValue;
}

API_AddOnCommandExecutionPolicy CommandBase::GetExecutionPolicy () const
{
    return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
}

bool CommandBase::IsProcessWindowVisible () const
{
    return false;
}

void CommandBase::OnResponseValidationFailed (const GS::ObjectState& /*response*/) const
{
}

ExportDotbimFileCommand::ExportDotbimFileCommand () :
    CommandBase ()
{
}

GS::String ExportDotbimFileCommand::GetName () const
{
    return "ExportDotbimFile";
}

GS::Optional<GS::UniString> ExportDotbimFileCommand::GetInputParametersSchema () const
{
    return R"({
        "type": "object",
        "properties": {
            "filePath": {
                "type": "string",
                "description": "The exported dotbim file path."
            }
        },
        "additionalProperties": false,
        "required": [
            "filePath"
        ]
    })";
}

GS::Optional<GS::UniString> ExportDotbimFileCommand::GetResponseSchema () const
{
    return R"({
        "type": "object",
        "properties": {
            "success": {
                "type": "boolean"
            },
            "error": {
                "type": "object",
                "properties": {
                    "code": {
                        "type": "integer"
                    },
                    "message": {
                        "type": "string"
                    }
                }
            }
        },
        "additionalProperties": false,
        "required": [
            "success"
        ]
    })";
}

GS::ObjectState ExportDotbimFileCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
    GS::UniString filePath;
    parameters.Get ("filePath", filePath);

    API_WindowInfo windowInfo = {};
    if (ACAPI_Window_GetCurrentWindow (&windowInfo) != NoError) {
        return CreateFailedExecutionResult (APIERR_GENERAL, "Internal error.");
    }

    if (windowInfo.typeID != APIWind_3DModelID) {
        API_WindowInfo newWindowInfo = {};
        newWindowInfo.typeID = APIWind_3DModelID;
        if (ACAPI_Window_ChangeWindow (&newWindowInfo) != NoError) {
            return CreateFailedExecutionResult (APIERR_GENERAL, "Failed to switch to 3D window.");
        }
    }

    IO::Location location (filePath);
    if (ExportDotbimFileFrom3DWindow (location) != NoError) {
        return CreateFailedExecutionResult (APIERR_GENERAL, "Failed export model.");
    }

    if (windowInfo.typeID != APIWind_3DModelID) {
        ACAPI_Window_ChangeWindow (&windowInfo);
    }

    return CreateSuccessfulExecutionResult ();
}

ImportDotbimFileCommand::ImportDotbimFileCommand () :
    CommandBase ()
{
}

GS::String ImportDotbimFileCommand::GetName () const
{
    return "ImportDotbimFile";
}

GS::Optional<GS::UniString> ImportDotbimFileCommand::GetInputParametersSchema () const
{
    return R"({
        "type": "object",
        "properties": {
            "filePath": {
                "type": "string",
                "description": "The exported dotbim file path."
            }
        },
        "additionalProperties": false,
        "required": [
            "filePath"
        ]
    })";
}

GS::Optional<GS::UniString> ImportDotbimFileCommand::GetResponseSchema () const
{
    return R"({
        "type": "object",
        "properties": {
            "success": {
                "type": "boolean"
            },
            "error": {
                "type": "object",
                "properties": {
                    "code": {
                        "type": "integer"
                    },
                    "message": {
                        "type": "string"
                    }
                }
            }
        },
        "additionalProperties": false,
        "required": [
            "success"
        ]
    })";
}

GS::ObjectState ImportDotbimFileCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
    GS::UniString filePath;
    parameters.Get ("filePath", filePath);

    GS::UniString commandString = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_MERGE_COMMAND_NAME, ACAPI_GetOwnResModule ());
    GSErrCode result = ACAPI_CallUndoableCommand (commandString, [&]() -> GSErrCode {
        IO::Location location (filePath);
        GS::UniString materialNameTemplate = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_MATERIAL_NAME, ACAPI_GetOwnResModule ());
        return ImportDotbim (location, materialNameTemplate);
    });

    if (result != NoError) {
        return CreateFailedExecutionResult (APIERR_GENERAL, "Failed import model.");
    }

    return CreateSuccessfulExecutionResult ();
}
