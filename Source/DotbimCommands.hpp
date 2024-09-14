#pragma once

#include <ACAPinc.h>

#include <ObjectState.hpp>

class CommandBase : public API_AddOnCommand
{
public:
    CommandBase ();

    virtual GS::String GetNamespace () const override;
    virtual GS::Optional<GS::UniString> GetSchemaDefinitions () const override;
    virtual API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override;
    virtual bool IsProcessWindowVisible () const override;
    virtual void OnResponseValidationFailed (const GS::ObjectState& response) const override;
};

class ExportDotbimFileCommand : public CommandBase
{
public:
    ExportDotbimFileCommand ();

    virtual GS::String GetName () const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;
    virtual GS::ObjectState Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};

class ImportDotbimFileCommand : public CommandBase
{
public:
    ImportDotbimFileCommand ();

    virtual GS::String GetName () const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;
    virtual GS::ObjectState Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};
