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

#include <transform_util/transform.h>

#include <boost/make_shared.hpp>

namespace transform_util
{
  Transform::Transform() :
    transform_(boost::make_shared<IdentityTransform>())
  {
  }

  Transform::Transform(const tf::Transform& transform) :
    transform_(boost::make_shared<TfTransform>(transform))
  {
  }
  
  Transform::Transform(const tf::StampedTransform& transform) :
    transform_(boost::make_shared<TfTransform>(transform))
  {
  }

  Transform::Transform(boost::shared_ptr<TransformImpl> transform) :
    transform_(transform)
  {
  }

  Transform& Transform::operator=(const tf::Transform transform)
  {
    transform_ = boost::make_shared<TfTransform>(transform);

    return *this;
  }

  Transform& Transform::operator=(boost::shared_ptr<TransformImpl> transform)
  {
    transform_ = transform;

    return *this;
  }

  tf::Vector3 Transform::operator()(const tf::Vector3& v) const
  {
    tf::Vector3 transformed;

    transform_->Transform(v, transformed);

    return transformed;
  }

  tf::Vector3 Transform::operator*(const tf::Vector3& v) const
  {
    tf::Vector3 transformed;

    transform_->Transform(v, transformed);

    return transformed;
  }

  tf::Vector3 Transform::GetOrigin() const
  {
    tf::Vector3 origin;

    transform_->Transform(tf::Vector3(0, 0, 0), origin);

    return origin;
  }

  tf::Quaternion Transform::GetOrientation() const
  {
    // Get the orientation of this transform by getting the vector between
    // the origin point and a point offset 1 on the x axis.

    tf::Vector3 offset;
    transform_->Transform(tf::Vector3(1, 0, 0), offset);

    tf::Vector3 vector = offset - GetOrigin();

    // Use the "half-way quaternion method" of summing and normalizing a
    // quaternion with twice the rotation between the vector and the x-axis and
    // the zero rotation.

    tf::Vector3 cross = tf::Vector3(1, 0, 0).cross(vector);
    double w = vector.length() + tf::Vector3(1, 0, 0).dot(vector);
    return tf::Quaternion(cross.x(), cross.y(), cross.z(), w).normalized();
  }

  void IdentityTransform::Transform(const tf::Vector3& v_in, tf::Vector3& v_out) const
  {
    v_out = v_in;
  }
  
  TfTransform::TfTransform(const tf::Transform& transform) :
    transform_(transform)
  {
    stamp_ = ros::Time::now();
  }
  
  TfTransform::TfTransform(const tf::StampedTransform& transform) :
    transform_(transform)
  {
    stamp_ = transform.stamp_;
  }

  void TfTransform::Transform(const tf::Vector3& v_in, tf::Vector3& v_out) const
  {
    v_out = transform_ * v_in;
  }
}
