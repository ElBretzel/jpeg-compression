#pragma once

#include "jpeg_body.hpp"
#include <math.h>

// See ITU-T81 A.3.3
void inverseDCT(std::unique_ptr<Body>& body);