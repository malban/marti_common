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

#include <cmath>
#include <cstdlib>

#include <gtest/gtest.h>

#include <transform_util/utm_util.h>

TEST(UtmUtilTests, GetZone)
{
  EXPECT_EQ(11, transform_util::GetZone(-118.408056));  // LAX
  EXPECT_EQ(17, transform_util::GetZone( -80.290556));  // MIA
  EXPECT_EQ(30, transform_util::GetZone(  -0.461389));  // LHR
  EXPECT_EQ(37, transform_util::GetZone(  37.414722));  // SVO
  EXPECT_EQ(54, transform_util::GetZone( 139.781111));  // HND
  EXPECT_EQ( 4, transform_util::GetZone(  -157.9225));  // HNL
}

TEST(UtmUtilTests, GetBand)
{
  EXPECT_EQ('S', transform_util::GetBand( 33.9425));    // LAX
  EXPECT_EQ('R', transform_util::GetBand( 25.793333));  // MIA
  EXPECT_EQ('U', transform_util::GetBand( 51.4775));    // LHR
  EXPECT_EQ('U', transform_util::GetBand( 55.972778));  // SVO
  EXPECT_EQ('S', transform_util::GetBand( 35.553333));  // HND
  EXPECT_EQ('Q', transform_util::GetBand( 21.318611));  // HNL
  EXPECT_EQ('F', transform_util::GetBand(-54.843333));  // USH

  EXPECT_EQ('Z', transform_util::GetBand(84.5));
  EXPECT_EQ('Z', transform_util::GetBand(-80.5));
}

TEST(UtmUtilTests, ToUtm)
{
  transform_util::UtmUtil utm_util;

  double easting, northing;
  int zone;
  char band;

  // LAX
  utm_util.ToUtm(33.9425, -118.408056, zone, band, easting, northing);
  EXPECT_EQ(11, zone);
  EXPECT_EQ('S', band);
  EXPECT_NEAR(369877, easting, 0.5);
  EXPECT_FLOAT_EQ(3756673, northing);

  utm_util.ToUtm(33.9425, -118.408056, easting, northing);
  EXPECT_NEAR(369877, easting, 0.5);
  EXPECT_FLOAT_EQ(3756673, northing);

  // MIA
  utm_util.ToUtm(25.793333, -80.290556, zone, band, easting, northing);
  EXPECT_EQ(17, zone);
  EXPECT_EQ('R', band);
  EXPECT_NEAR(571124, easting, 0.5);
  EXPECT_FLOAT_EQ(2852989, northing);

  utm_util.ToUtm(25.793333, -80.290556, easting, northing);
  EXPECT_NEAR(571124, easting, 0.5);
  EXPECT_FLOAT_EQ(2852989, northing);

  // USH
  utm_util.ToUtm(-54.843333, -68.295556, zone, band,easting, northing);
  EXPECT_EQ(19, zone);
  EXPECT_EQ('F', band);
  EXPECT_FLOAT_EQ(545237, easting);
  EXPECT_FLOAT_EQ(3922415, northing);

  utm_util.ToUtm(-54.843333, -68.295556, easting, northing);
  EXPECT_FLOAT_EQ(545237, easting);
  EXPECT_FLOAT_EQ(3922415, northing);
}

TEST(UtmUtilTests, ToWgs84)
{
  transform_util::UtmUtil utm_util;

  double lat, lon;

  // LAX
  utm_util.ToLatLon(11, 'S', 369877, 3756673, lat, lon);
  EXPECT_FLOAT_EQ(33.9425, lat);
  EXPECT_NEAR(-118.408056, lon, .000005);

  // LAX
  utm_util.ToLatLon(17, 'R', 571124, 2852989, lat, lon);
  EXPECT_FLOAT_EQ(25.793333, lat);
  EXPECT_NEAR(-80.290556, lon, .000005);

  // USH
  utm_util.ToLatLon(19, 'F', 545237, 3922415, lat, lon);
  EXPECT_FLOAT_EQ(-54.843333, lat);
  EXPECT_FLOAT_EQ(-68.295556, lon);
}

TEST(UtmUtilTests, Continuity)
{
  transform_util::UtmUtil utm_util;

  // (FOR) - Fortaleza International Airport
  double easting = 551940;
  double northing = 9582637;

  double last_lon = 0;

  for (int i = 0; i < 1000; i++)
  {
    double new_lat;
    double new_lon;
    double new_easting;
    double new_northing;
    int zone;
    char band;
    utm_util.ToLatLon(24, 'M', easting + (double)i * 1.11 / 100.0, northing, new_lat, new_lon);
    utm_util.ToUtm(new_lat, new_lon, zone, band, new_easting, new_northing);

    EXPECT_FLOAT_EQ(easting + (double)i * 1.11 / 100.0, new_easting);
    EXPECT_FLOAT_EQ(northing, new_northing);

    if (i > 0)
    {
      // The difference should be 1.11cm which is approximately
      // 1/10th of 1 microdegree near the equator
      EXPECT_NEAR(0.0000001, std::fabs(new_lon - last_lon), 0.00000001);
    }

    last_lon = new_lon;
  }
}

TEST(UtmUtilTests, Random)
{
  transform_util::UtmUtil utm_util;

  std::srand(0);

  for (int i = 0; i < 1000; i++)
  {
    double lon = ((double)std::rand() / RAND_MAX) * 360.0 - 180;
    double lat = ((double)std::rand() / RAND_MAX) * 140.0 - 70;

    char band;
    int zone;
    double easting;
    double northing;
    utm_util.ToUtm(lat, lon, zone, band, easting, northing);

    double new_lat;
    double new_lon;
    utm_util.ToLatLon(zone, band, easting, northing, new_lat, new_lon);

    EXPECT_FLOAT_EQ(lat, new_lat);
    EXPECT_FLOAT_EQ(lon, new_lon);
  }
}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}