// *****************************************************************************
//
// Copyright (C) 2013 All Right Reserved, Southwest Research Institute® (SwRI®)
//
// Contract No.  10-R8248
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

#ifndef TRANSFORM_UTIL_TRANSFORM_UTIL_H_
#define TRANSFORM_UTIL_TRANSFORM_UTIL_H_

#include <boost/array.hpp>

#include <tf/transform_datatypes.h>

#include <transform_util/local_xy_util.h>

namespace transform_util
{
  tf::Transform GetRelativeTransform(
      double latitude,
      double longitude,
      double yaw,
      double reference_latitude,
      double reference_longitude,
      double reference_yaw);

  double GetBearing(
      double source_latitude,
      double source_longitude,
      double destination_latitude,
      double destination_longitude);

  /**
   * Snaps a quaternion rotation to the closest right angle rotation.
   *
   * @param[in]  rotation  The input quaternion rotation.
   *
   * @returns The closest right angle rotation to the input rotation.
   */
  tf::Quaternion SnapToRightAngle(const tf::Quaternion& rotation);

  /**
   * Return an axis aligned unit vector that is nearest the provided vector.
   *
   * @param[in]  vector  The input vector.
   *
   * @returns The axis aligned unit vector.
   */
  tf::Vector3 GetPrimaryAxis(const tf::Vector3& vector);

  /**
   * Validate that a 3x3 matrix is a rotation.
   *
   * @param[in]  matrix  The matrix to validate.
   *
   * @returns True if the matrix is a valid rotation.
   */
  bool IsRotation(tf::Matrix3x3 matrix);

  /**
   * Gets the upper-left 3x3 sub-matrix of a 6x6 matrix.
   *
   * @param[in]  matrix  The 6x6 matrix.
   *
   * @returns The upper-left 3x3 sub-matrix.
   */
  tf::Matrix3x3 GetUpperLeft(const boost::array<double, 36>& matrix);

  /**
   * Gets the lower-right 3x3 sub-matrix of a 6x6 matrix.
   *
   * @param[in]  matrix  The 6x6 matrix.
   *
   * @returns The lower-right 3x3 sub-matrix.
   */
  tf::Matrix3x3 GetLowerRight(const boost::array<double, 36>& matrix);

  /**
   * Sets the upper-left quadrant of a 6x6 matrix with the specified 3x3
   * sub-matrix
   *
   * @param[in]  sub_matrix  The 3x3 sub-matrix.
   * @param[out] matrix      The 6x6 matrix to modify.
   */
  void SetUpperLeft(
      const tf::Matrix3x3& sub_matrix,
      boost::array<double, 36>& matrix);

  /**
   * Sets the lower-right quadrant of a 6x6 matrix with the specified 3x3
   * sub-matrix
   *
   * @param[in]  sub_matrix  The 3x3 sub-matrix.
   * @param[out] matrix      The 6x6 matrix to modify.
   */
  void SetLowerRight(
      const tf::Matrix3x3& sub_matrix,
      boost::array<double, 36>& matrix);
}

#endif  // TRANSFORM_UTIL_TRANSFORM_UTIL_H_
