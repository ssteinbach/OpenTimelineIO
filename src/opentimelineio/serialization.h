#pragma once

#include <string>
#include "opentimelineio/any.h"
#include "opentimelineio/errorStatus.h"

std::string serialize_json_to_string(const any& value, ErrorStatus* error_status, int indent = 4);

bool serialize_json_to_file(const any& value, std::string const& file_name,
                            ErrorStatus* error_status, int indent = 4);
