#include "GSGuid.hpp"

#include <string>
#include <functional>

using PropertyProcessor = std::function<void (const std::string&, const std::string&)>;

void EnumerateProjectProperties (const PropertyProcessor& processor);
void EnumerateElemProperties (const GS::Guid& elemGuid, const PropertyProcessor& processor);
