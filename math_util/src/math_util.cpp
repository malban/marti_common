// *****************************************************************************
//
// Copyright (C) 2013 All Right Reserved, Southwest Research Institute® (SwRI®)
//
// Contract No.  10-62987
// Contractor    Southwest Research Institute® (SwRI®)
// Address       6220 Culebra Road, San Antonio, Texas 78228-0510
// Contact       Kris Kozak <kkozak@swri.org> (210) 522-3854
//
// This code was developed as part of an internal research project fully funded
// by Southwest Research Institute®.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// *****************************************************************************

#include <math_util/math_util.h>

#include <cmath>

namespace math_util
{
  double Round(double value)
  {
    return (value > 0.0) ? std::floor(value + 0.5) : std::ceil(value - 0.5);
  }

  double ToNearest(double value, double multiple)
  {
    if (multiple == 0)
    {
      return 0;
    }

    return Round(value / multiple) * multiple;
  }

  bool IsNear(double v1, double v2, double epsilon)
  {
    return std::fabs(v1 - v2) <= epsilon;
  }
}