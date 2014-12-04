// *****************************************************************************
//
// Copyright (C) 2014 All Right Reserved, Southwest Research Institute® (SwRI®)
//
// Contractor    Southwest Research Institute® (SwRI®)
// Address       6220 Culebra Road, San Antonio, Texas 78228-0510
// Contact       Kris Kozak <kkozak@swri.org> (210) 522-3854
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// *****************************************************************************

#ifndef STRING_UTIL_STRING_UTIL_H_
#define STRING_UTIL_STRING_UTIL_H_

#include <stdint.h>
#include <string>

namespace string_util
{
  bool ToDouble(const std::string& string, double& value);

  bool ToFloat(const std::string& string, float& value);

  bool ToInt32(const std::string& string, int32_t& value, int32_t base = 10);

  bool ToUInt32(const std::string& string, uint32_t& value, int32_t base = 10);
}

#endif  // STRING_UTIL_STRING_UTIL_H_