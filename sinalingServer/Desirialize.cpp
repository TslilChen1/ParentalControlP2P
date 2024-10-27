#include "Desirialize.h"

const AuthenticationRequest Desirialize::deserializeAuthentication(nlohmann::json json_obj)
{
	AuthenticationRequest r;
    r.id = json_obj["id"];
    r.username = json_obj["username"];
    r.password = json_obj["password"];
    r.role = json_obj["role"];
    r.ip = json_obj["ip"];
    return r;
}

