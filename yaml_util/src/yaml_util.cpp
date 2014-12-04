// *****************************************************************************
//
// Copyright (c) 2014, Southwest Research Institute® (SwRI®)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *****************************************************************************

#include <yaml_util/yaml_util.h>

// C++ standard libraries
#ifdef YAMLCPP_OLD_API
#include <fstream>
#endif  // YAMLCPP_OLD_API

#ifndef YAMLCPP_OLD_API
namespace YAML
{
  void operator >> (const YAML::Node& node, float& value)
  {
    value = node.as<float>();
  }

  void operator >> (const YAML::Node& node, double& value)
  {
    value = node.as<double>();
  }

  void operator >> (const YAML::Node& node, bool& value)
  {
    value = node.as<bool>();
  }

  void operator >> (const YAML::Node& node, int16_t& value)
  {
    value = node.as<int16_t>();
  }

  void operator >> (const YAML::Node& node, int32_t& value)
  {
    value = node.as<int32_t>();
  }

  void operator >> (const YAML::Node& node, uint32_t& value)
  {
    value = node.as<uint32_t>();
  }

  void operator >> (const YAML::Node& node, int64_t& value)
  {
    value = node.as<int64_t>();
  }
  
  void operator >> (const YAML::Node& node, uint64_t& value)
  {
    value = node.as<uint64_t>();
  }

  void operator >> (const YAML::Node& node, std::string& value)
  {
    value = node.as<std::string>();
  }
}
#endif  // YAMLCPP_OLD_API

namespace yaml_util
{
  bool LoadFile(const std::string& path, YAML::Node& yaml)
  {
    try
    {
      #ifndef YAMLCPP_OLD_API
      yaml = YAML::LoadFile(path);
      #else
      std::ifstream fin(path.c_str());
      if (fin.fail())
      {
        return false;
      }
      
      YAML::Parser parser(fin);
      parser.GetNextDocument(yaml);
      #endif  // YAMLCPP_OLD_API
    }
    catch (...)
    {
      return false;
    }
    
    return true;
  }
  
  bool FindValue(const YAML::Node& node, const std::string& name)
  {
    #ifndef YAMLCPP_OLD_API
      return node[name];
    #else
      return node.FindValue(name);
    #endif  // YAMLCPP_OLD_API
  }
}
