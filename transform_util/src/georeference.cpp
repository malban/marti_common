// *****************************************************************************
//
// Copyright (C) 2013 All Right Reserved, Southwest Research Institute® (SwRI®)
//
// Contract No.  10-58058A
// Contractor    Southwest Research Institute® (SwRI®)
// Address       6220 Culebra Road, San Antonio, Texas 78228-0510
// Contact       Steve Dellenback <sdellenback@swri.org> (210) 522-3914
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

#include <transform_util/georeference.h>

// C++ standard libraries
#include <fstream>

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>

// ROS libraries
#include <ros/ros.h>

namespace transform_util
{
  GeoReference::GeoReference(const std::string& path) :
    loaded_(false),
    path_(path),
    image_path_(""),
    width_(0),
    height_(0),
    tile_size_(0),
    datum_(""),
    projection_(""),
    transform_(2, 3, CV_64F),
    pixels_(1, 1, CV_32SC2),
    coordinates_(1, 1, CV_64FC2),
    x_offset_(0),
    y_offset_(0)
  {
    // Initialize transform to identity
    transform_.at<double>(0, 0) = 1;
    transform_.at<double>(0, 1) = 0;
    transform_.at<double>(0, 2) = 0;
    transform_.at<double>(1, 0) = 0;
    transform_.at<double>(1, 1) = 1;
    transform_.at<double>(1, 2) = 0;
  }

  GeoReference::GeoReference(const GeoReference& geo) :
    loaded_(geo.loaded_),
    path_(geo.path_),
    image_path_(geo.image_path_),
    width_(geo.width_),
    height_(geo.height_),
    tile_size_(geo.tile_size_),
    datum_(geo.datum_),
    projection_(geo.projection_),
    transform_(geo.transform_)
  {
  }

  GeoReference::~GeoReference()
  {
  }

  bool GeoReference::Load()
  {
    if (loaded_)
      return true;

    std::ifstream fin(path_.c_str());
    if (fin.fail())
    {
      ROS_ERROR("Failed to load file: %s", path_.c_str());
      return false;
    }

    try
    {
      YAML::Parser parser(fin);

      YAML::Node doc;
      parser.GetNextDocument(doc);

      doc["image_path"] >> image_path_;

      boost::filesystem::path imagePath(image_path_);

      // If we have an absolute path, we don't need to change it
      // Otherwise, we want to create a relative path from the .geo file
      // location
      if (imagePath.is_complete() == false)
      {
        boost::filesystem::path geoPath(path_);
        image_path_ = (geoPath.parent_path() / imagePath.relative_path()).normalize().string();

        ROS_INFO("georeference: Image path is %s", image_path_.c_str());
      }

      doc["image_width"] >> width_;
      doc["image_height"] >> height_;
      doc["tile_size"] >> tile_size_;

      doc["datum"] >> datum_;
      doc["projection"] >> projection_;

      // Parse in the tiepoints
      pixels_ = cv::Mat(1, doc["tiepoints"].size(), CV_32SC2);
      coordinates_ = cv::Mat(1, doc["tiepoints"].size(), CV_64FC2);
      ROS_INFO("georeference: Found %d tiepoints", (int32_t)(doc["tiepoints"].size()));
      for (unsigned int i = 0; i < doc["tiepoints"].size(); i++)
      {
        // Parse pixel column value into the pixel list
        doc["tiepoints"][i]["point"][0] >> pixels_.at<cv::Vec2s>(0, i)[0];

        // Parse pixel row value into the pixel list
        doc["tiepoints"][i]["point"][1] >> pixels_.at<cv::Vec2s>(0, i)[1];

        // Parse the x coordinate into the coordinate list
        doc["tiepoints"][i]["point"][2] >> coordinates_.at<cv::Vec2d>(0, i)[0];

        // Parse the y coordinate into the coordinate list
        doc["tiepoints"][i]["point"][3] >> coordinates_.at<cv::Vec2d>(0, i)[1];
      }

      if (doc["tiepoints"].size() > 1)
      {
        GetTransform();
      }
      else if (doc["tiepoints"].size() == 1)
      {
        // Parse in the X scale
        doc["pixel_scale"][0] >> transform_.at<float>(0, 0);

        // Parse in the Y scale
        doc["pixel_scale"][1] >> transform_.at<float>(1, 1);

        transform_.at<float>(0, 2) = coordinates_.at<cv::Vec2d>(0, 1)[0] -
            pixels_.at<cv::Vec2s>(0, 1)[0] * transform_.at<double>(0, 0);

        transform_.at<float>(1, 2) = coordinates_.at<cv::Vec2d>(0, 1)[1] -
            pixels_.at<cv::Vec2s>(0, 1)[1] * transform_.at<double>(1, 1);
      }
      else
      {
        ROS_ERROR("georeference: No tiepoints.");
        return false;
      }

      Print();
    }
    catch (const YAML::ParserException& e)
    {
      ROS_ERROR("%s", e.what());
      return false;
    }
    catch (const YAML::Exception& e)
    {
      ROS_ERROR("%s", e.what());
      return false;
    }

    loaded_ = true;
    return true;
  }

  void GeoReference::GetTransform()
  {
    // Copy pixels into float 32 matrix
    cv::Mat src(1, pixels_.cols, CV_32FC2);
    for (int i = 0; i < pixels_.cols; i++)
    {
      src.at<cv::Vec2f>(0, i)[0] = static_cast<float>(pixels_.at<cv::Vec2s>(0, i)[0]);
      src.at<cv::Vec2f>(0, i)[1] = static_cast<float>(pixels_.at<cv::Vec2s>(0, i)[1]);
    }

    // Offset coordinates to a new origin to avoid loss of precision
    x_offset_ = coordinates_.at<cv::Vec2d>(0, 0)[0];
    y_offset_ = coordinates_.at<cv::Vec2d>(0, 0)[1];

    // Copy coordinates into float 32 matrix
    cv::Mat dst(1, pixels_.cols, CV_32FC2);
    for (int i = 0; i < pixels_.cols; i++)
    {
      dst.at<cv::Vec2f>(0, i)[0] = coordinates_.at<cv::Vec2d>(0, i)[0] - x_offset_;
      dst.at<cv::Vec2f>(0, i)[1] = coordinates_.at<cv::Vec2d>(0, i)[1] - y_offset_;
    }

    transform_ = cv::estimateRigidTransform(src, dst, true);
    inverse_transform_ = cv::estimateRigidTransform(dst, src, true);
  }

  void GeoReference::GetCoordinate(int x_pixel, int y_pixel, double& x_coordinate, double& y_coordinate) const
  {
    cv::Mat src(1, 1, CV_32FC2);
    cv::Mat dst(1, 1, CV_32FC2);

    src.at<cv::Vec2f>(0, 0)[0] = x_pixel;
    src.at<cv::Vec2f>(0, 0)[1] = y_pixel;

    cv::transform(src, dst, transform_);

    x_coordinate = dst.at<cv::Vec2f>(0, 0)[0] + x_offset_;
    y_coordinate = dst.at<cv::Vec2f>(0, 0)[1] + y_offset_;
  }

  void GeoReference::GetPixel(double x_coordinate, double y_coordinate, int& x_pixel, int& y_pixel) const
  {
    cv::Mat src(1, 1, CV_32FC2);
    cv::Mat dst(1, 1, CV_32FC2);

    src.at<cv::Vec2f>(0, 0)[0] = x_coordinate - x_offset_;
    src.at<cv::Vec2f>(0, 0)[1] = y_coordinate - y_offset_;

    cv::transform(src, dst, inverse_transform_);

    x_pixel = static_cast<int>(dst.at<cv::Vec2f>(0, 0)[0]);
    y_pixel = static_cast<int>(dst.at<cv::Vec2f>(0, 0)[1]);
  }

  void GeoReference::Print()
  {
    ROS_INFO("georeference:  path = %s", path_.c_str());
    ROS_INFO("georeference:  image = %s", image_path_.c_str());
    ROS_INFO("georeference:  width = %d", width_);
    ROS_INFO("georeference:  height = %d", height_);
    ROS_INFO("georeference:  tile_size = %d", tile_size_);
    ROS_INFO("georeference:  datum = %s", datum_.c_str());
    ROS_INFO("georeference:  projection = %s", projection_.c_str());

    ROS_INFO("georeference:  tiepoints");
    for (int i = 0; i < pixels_.cols; i++)
    {
      ROS_INFO("georeference:     [%d, %d, %lf, %lf]",
          pixels_.at<cv::Vec2s>(0, i)[0],
          pixels_.at<cv::Vec2s>(0, i)[1],
          coordinates_.at<cv::Vec2d>(0, i)[0],
          coordinates_.at<cv::Vec2d>(0, i)[1]);
    }

    ROS_INFO("georeference:  transform: %8lf, %8lf, %8lf",
        transform_.at<double>(0, 0),
        transform_.at<double>(0, 1),
        transform_.at<double>(0, 2) + x_offset_);

    ROS_INFO("georeference:             %8lf, %8lf, %8lf",
        transform_.at<double>(1, 0),
        transform_.at<double>(1, 1),
        transform_.at<double>(1, 2) + y_offset_);

    ROS_INFO("georeference:             %8lf, %8lf, %8lf", 0.0, 0.0, 1.0);
  }
}

