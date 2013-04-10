// *****************************************************************************
//
// Copyright (C) 2012 All Right Reserved, Southwest Research Institute® (SwRI®)
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

#include <image_util/geometry_util.h>

#include <cmath>
#include <vector>

#include <QPolygonF>
#include <QPointF>

#include <opencv2/imgproc/imgproc.hpp>

#include <lapackpp.h>

#include <math_util/constants.h>

namespace image_util
{
  bool Intersects(const BoundingBox& box1, const BoundingBox& box2)
  {
    // Create points for each corner of box 1
    cv::Point_<double> n1(box1.x, box1.y);
    cv::Point_<double> n2(box1.x + box1.width, box1.y + box1.height);
    cv::Point_<double> n3(box1.x, box1.y + box1.height);
    cv::Point_<double> n4(box1.x + box1.width, box1.y);

    // Create points for each corner of box 2
    cv::Point_<double> m1(box2.x, box2.y);
    cv::Point_<double> m2(box2.x + box2.width, box2.y + box2.height);
    cv::Point_<double> m3(box2.x, box2.y + box2.height);
    cv::Point_<double> m4(box2.x + box2.width, box2.y);

    // Return true if any of the corners of one box are contained within the
    // other box.
    return box1.contains(m1) ||
           box1.contains(m2) ||
           box1.contains(m3) ||
           box1.contains(m4) ||
           box2.contains(n1) ||
           box2.contains(n2) ||
           box2.contains(n3) ||
           box2.contains(n4);
  }

  double GetOverlappingArea(const cv::Rect& rect, const cv::Mat& rigid_transform)
  {
    // List of points corresponding to the input rectangle.
    std::vector<cv::Vec2f> points;

    // List of points correspondng to the transformed rectangle.
    std::vector<cv::Vec2f> points_t;

    // Create a point for each corner of the input rectangle.
    points.push_back(cv::Vec2f(rect.x, rect.y));
    points.push_back(cv::Vec2f(rect.x + rect.width, rect.y));
    points.push_back(cv::Vec2f(rect.x + rect.width, rect.y + rect.height));
    points.push_back(cv::Vec2f(rect.x, rect.y + rect.height));

    // Transform the input points to the transformed points using the rigid
    // transform.
    cv::transform(cv::InputArray(points), cv::OutputArray(points_t), rigid_transform);

    // Use the QPolygon object to get the intersecting area of the input
    // rectangle and the transformed rectangle.

    // Build the polygon corresponding to the input rectangle.
    QPolygonF polygon;
    polygon << QPointF(points[0][0], points[0][1]);
    polygon << QPointF(points[1][0], points[1][1]);
    polygon << QPointF(points[2][0], points[2][1]);
    polygon << QPointF(points[3][0], points[3][1]);
    polygon << QPointF(points[0][0], points[0][1]);

    // Build the polygon corresponding to the transformed rectangle.
    QPolygonF transformed_polygon;
    transformed_polygon << QPointF(points_t[0][0], points_t[0][1]);
    transformed_polygon << QPointF(points_t[1][0], points_t[1][1]);
    transformed_polygon << QPointF(points_t[2][0], points_t[2][1]);
    transformed_polygon << QPointF(points_t[3][0], points_t[3][1]);
    transformed_polygon << QPointF(points_t[0][0], points_t[0][1]);

    // Get the polygon representing the intersection of the input rectangle and
    // the transformed rectangle.
    QPolygonF intersection = polygon.intersected(transformed_polygon);

    // If the intersection is empty, then just return 0 area.
    if (intersection.size() == 0)
    {
      return 0;
    }

    // Build an OpenCV contour to measure the area of the intersection.
    std::vector<cv::Point2f> contour;
    for (int i = 0; i < intersection.size(); i++)
    {
      contour.push_back(cv::Point2f(intersection[i].x(), intersection[i].y()));
    }

    // Scale the area based on the scale factor to get the correct value.
    return cv::contourArea(contour);
  }

  cv::Mat ProjectEllipsoid(const cv::Mat& ellipsoid)
  {
    cv::Mat ellipse;

    if (ellipsoid.rows == 3 && ellipsoid.cols == 3 && ellipsoid.type() == CV_32FC1)
    {
      LaGenMatDouble A(3, 3);
      for (int32_t r = 0; r < 3; r++)
      {
        for (int32_t c = 0; c < 3; c++)
        {
          A(r, c) = ellipsoid.at<float>(r, c);
        }
      }

      // Specify the main vector directions (x and y)
      LaVectorDouble ax1 = LaVectorDouble::zeros(3, 1);
      LaVectorDouble ax2 = LaVectorDouble::zeros(3, 1);
      ax1(0) = 1;
      ax2(1) = 1;

      // Initialize the normal vector (here the normal vector, in the state
      // space is in the theta direction).
      LaVectorDouble n_ax = LaVectorDouble::zeros(3, 1);
      n_ax(2) = 1;

      ///////////////////////
      // Calculate A prime //
      ///////////////////////

      LaGenMatDouble A_sym_temp = A;
      LaGenMatDouble temp1 = LaGenMatDouble::eye(A.rows(), A.cols());
      Blas_Mat_Trans_Mat_Mult(A, temp1, A_sym_temp, 1.0, 1.0);

      // N_ax_temp = (n_ax*n_ax')
      LaGenMatDouble N_ax_temp = LaGenMatDouble::zeros(3, 3);
      Blas_R1_Update(N_ax_temp, n_ax, n_ax);

      // A_prime_1 = A_sym_temp*N_ax_temp
      //           = (A+A')*(n_ax*n_ax')
      LaGenMatDouble A_prime_1 = LaGenMatDouble::zeros(3, 3);
      Blas_Mat_Mat_Mult(A_sym_temp, N_ax_temp, A_prime_1);

      // A_prime_2 = A_prime_1*A_sym_temp
      //           = (A+A')*(n_ax*n_ax')*(A+A')
      LaGenMatDouble A_prime_2 = LaGenMatDouble::zeros(3, 3);
      Blas_Mat_Mat_Mult(A_prime_1, A_sym_temp, A_prime_2);

      // scale_1 = n_ax'*A
      LaGenMatDouble scale_1(1, 3);
      Blas_Mat_Trans_Mat_Mult(n_ax, A, scale_1);

      // scale_2 = (scale_1*n_ax)*-4
      //         = (n_ax'*A*n_ax)*-4
      LaGenMatDouble scale_2(1, 1);
      Blas_Mat_Mat_Mult(scale_1, n_ax, scale_2);
      scale_2.scale(-4.0);

      // Convert from matrix to scalar
      double scale = scale_2(0, 0);

      // A_temp = scale*A_temp
      //        = scale_2*A = -4*(n_ax'*A*n_ax)*A
      LaGenMatDouble A_temp = A;
      A_temp.scale(scale);

      // Aprime = A_prime_2 + A_temp
      //        = (A+A')*(n_ax*n_ax')*(A+A') - 4*(n_ax'*A*n_ax)*A
      LaGenMatDouble Aprime(3, 3);
      Aprime = A_prime_2 + A_temp;

      ///////////////////////
      // Calculate C prime //
      ///////////////////////

      // C_temp = n_ax'*A
      LaGenMatDouble C_temp(1, 3);
      Blas_Mat_Trans_Mat_Mult(n_ax, A, C_temp);

      // Cprime = -4.0*C_temp*n_ax
      //        = -4.0*n_ax'*A*n_ax
      LaGenMatDouble Cprime(1, 1);
      Blas_Mat_Mat_Mult(C_temp, n_ax, Cprime, -4.0);

      double cp = Cprime(0, 0);

      // Bprime = Aprime/Cprime;
      LaGenMatDouble Bprime = Aprime;
      Bprime.scale(1.0 / cp);

      // Jp = axes_def;
      //    = [ax1(:),ax2(:)] = [1,0;0,1;0,0];
      LaGenMatDouble Jp = LaGenMatDouble::eye(3, 2);

      ///////////////////////
      // Calculate D prime //
      ///////////////////////

      // Dprime_temp = Jp'*Bprime
      LaGenMatDouble Dprime_temp(2, 3);
      Blas_Mat_Trans_Mat_Mult(Jp, Bprime, Dprime_temp);

      // Dprime = Dprime_temp * Jp
      //        = Jp'*Bprime*Jp
      LaGenMatDouble Dprime(2, 2);
      Blas_Mat_Mat_Mult(Dprime_temp, Jp, Dprime);

      for (int32_t r = 0; r < 2; r++)
      {
        for (int32_t c = 0; c < 2; c++)
        {
          if (Dprime(r, c) != Dprime(r, c))
          {
            return ellipse;
          }
        }
      }

      ellipse.create(2, 2, CV_32FC1);
      ellipse.at<float>(0, 0) = Dprime(0, 0);
      ellipse.at<float>(0, 1) = Dprime(0, 1);
      ellipse.at<float>(1, 0) = Dprime(1, 0);
      ellipse.at<float>(1, 1) = Dprime(1, 1);
    }

    return ellipse;
  }

  std::vector<tf::Vector3> GetEllipsePoints(
      const cv::Mat& ellipse,
      const tf::Vector3& center,
      double scale,
      int32_t num_points)
  {
    std::vector<tf::Vector3> perimeter;

    if (ellipse.rows == 2 && ellipse.cols == 2 && ellipse.type() == CV_32FC1 &&
        num_points > 2)
    {
      LaGenMatDouble Dprime(2, 2);
      Dprime(0, 0) = ellipse.at<float>(0, 0);
      Dprime(0, 1) = ellipse.at<float>(0, 1);
      Dprime(1, 0) = ellipse.at<float>(1, 0);
      Dprime(1, 1) = ellipse.at<float>(1, 1);

      LaVectorDouble Sigma(2);
      LaGenMatDouble U(2, 2);
      LaGenMatDouble Vt(2, 2);

      LaSVD_IP(Dprime, Sigma, U, Vt);

      double xprime_scale = sqrt(Sigma(0));
      double yprime_scale = sqrt(Sigma(1));

      if (xprime_scale <= 0 || yprime_scale <= 0)
      {
        return perimeter;
      }

      LaGenMatDouble Xp1 = LaGenMatDouble::zeros(num_points, 2);
      for (int32_t i = 0; i < num_points; i++)
      {
        double phi =
            (static_cast<double>(i) / static_cast<double>(num_points))
            * math_util::_2pi;

        Xp1(i, 0) = xprime_scale * std::cos(phi) * scale;
        Xp1(i, 1) = yprime_scale * std::sin(phi) * scale;
      }

      LaGenMatDouble Xell(num_points, 2);

      // Xell = Xp1*(V')'
      Blas_Mat_Mat_Trans_Mult(Xp1, Vt, Xell);

      perimeter.resize(num_points);
      for (int32_t i = 0; i < num_points; i++)
      {
        perimeter[i].setX(Xell(i, 0) + center.x());
        perimeter[i].setY(Xell(i, 1) + center.y());
      }
    }

    return perimeter;
  }
}
