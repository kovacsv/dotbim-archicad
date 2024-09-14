#pragma once

#include <Model.hpp>

std::string ExportDotbim (const ModelerAPI::Model& model);
GSErrCode ExportDotbimFile (const ModelerAPI::Model& model, const IO::Location& location);
GSErrCode ExportDotbimFileFrom3DWindow (const IO::Location& location);
