#pragma once
#include "Requests.h"
#include "json.hpp"

class Desirialize
{
public:

    const static AuthenticationRequest deserializeAuthentication(nlohmann::json json_obj);

};

