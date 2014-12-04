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

#include <transform_util/wgs84_transformer.h>

#include <boost/make_shared.hpp>

#include <transform_util/frames.h>

#include <pluginlib/class_list_macros.h>
PLUGINLIB_DECLARE_CLASS(
    transformers,
    wgs84,
    transform_util::Wgs84Transformer,
    transform_util::Transformer)

namespace transform_util
{
  Wgs84Transformer::Wgs84Transformer()
  {
  }

  std::map<std::string, std::vector<std::string> > Wgs84Transformer::Supports() const
  {
    std::map<std::string, std::vector<std::string> >  supports;

    supports[_wgs84_frame].push_back(_tf_frame);
    supports[_tf_frame].push_back(_wgs84_frame);

    return supports;
  }

  bool Wgs84Transformer::GetTransform(
    const std::string& target_frame,
    const std::string& source_frame,
    const ros::Time& time,
    Transform& transform)
  {
    if (!initialized_)
    {
      Initialize();
    }

    if (!initialized_)
    {
      ROS_ERROR("Wgs84Transformer not initialized");
      return false;
    }

    if (target_frame == _wgs84_frame)
    {
      tf::StampedTransform tf_transform;
      if (!Transformer::GetTransform(local_xy_frame_, source_frame , time, tf_transform))
      {
        ROS_ERROR("Failed to get transform between %s and %s",
            source_frame.c_str(), local_xy_frame_.c_str());
        return false;
      }

      transform = boost::make_shared<TfToWgs84Transform>(tf_transform, local_xy_util_);

      return true;
    }
    else if (source_frame == _wgs84_frame)
    {
      tf::StampedTransform tf_transform;
      if (!Transformer::GetTransform(target_frame, local_xy_frame_, time, tf_transform))
      {
        ROS_ERROR("Failed to get transform between %s and %s",
            local_xy_frame_.c_str(), target_frame.c_str());
        return false;
      }

      transform = boost::make_shared<Wgs84ToTfTransform>(tf_transform, local_xy_util_);

      return true;
    }

    ROS_ERROR("Failed to get WGS84 transform.");
    return false;
  }

  bool Wgs84Transformer::Initialize()
  {
    if (!ros::param::get("/local_xy_frame", local_xy_frame_))
    {
      ROS_ERROR("Failed to parse /local_xy_frame");
      return false;
    }

    if (!local_xy_util_)
    {
      local_xy_util_ = boost::make_shared<LocalXyWgs84Util>();
    }

    initialized_ = local_xy_util_->Initialized();

    return initialized_;
  }
  
  TfToWgs84Transform::TfToWgs84Transform(
    const tf::StampedTransform& transform,
    boost::shared_ptr<LocalXyWgs84Util> local_xy_util) :
    transform_(transform),
    local_xy_util_(local_xy_util)
  {
    stamp_ = transform.stamp_;
  }

  void TfToWgs84Transform::Transform(const tf::Vector3& v_in, tf::Vector3& v_out) const
  {
    // Transform into the LocalXY coordinate frame using the TF transform.
    tf::Vector3 local_xy = transform_ * v_in;

    // Convert to WGS84 latitude and longitude.
    double latitude, longitude;
    local_xy_util_->ToWgs84(local_xy.x(), local_xy.y(), latitude, longitude);
    v_out.setValue(longitude, latitude, local_xy.z());
  }
  
  Wgs84ToTfTransform::Wgs84ToTfTransform(
    const tf::StampedTransform& transform,
    boost::shared_ptr<LocalXyWgs84Util> local_xy_util) :
    transform_(transform),
    local_xy_util_(local_xy_util)
  {
    stamp_ = transform.stamp_;
  }

  void Wgs84ToTfTransform::Transform(const tf::Vector3& v_in, tf::Vector3& v_out) const
  {
    // Convert to LocalXY coordinate frame.
    double x, y;
    local_xy_util_->ToLocalXy(v_in.y(), v_in.x(), x, y);
    v_out.setValue(x, y, v_in.z());

    // Transform from the LocalXY coordinate frame using the TF transform.
    v_out = transform_ * v_out;
  }
}

