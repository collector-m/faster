#ifndef UTILS_HPP
#define UTILS_HPP
#include <iostream>
#include "ros/ros.h"
#include <std_msgs/ColorRGBA.h>
#include <geometry_msgs/Vector3.h>
#include <geometry_msgs/Point.h>
// TODO: This values should be the same as the global_mapper.yaml
#define WDX 20    //[m] world dimension in x
#define WDY 20    //[m] world dimension in y
#define WDZ 4     //[m] world dimension in z
#define RES 0.15  //[m] cell dimension

#define RED 1
#define GREEN 2
#define BLUE 3
#define BLUE_LIGHT 4
#define YELLOW 5
#define ORANGE_TRANS 6

#define DC 0.01           //(seconds) Duration for the interpolation=Value of the timer pubGoal
#define GOAL_RADIUS 0.2   //(m) Drone has arrived to the goal when distance_to_goal<GOAL_RADIUS
#define DRONE_RADIUS 0.3  //(m) Used for collision checking

#define STATE 0
#define INPUT 1

#define V_MAX 2
#define A_MAX 2
#define J_MAX 10

#define POS 0
#define VEL 1
#define ACCEL 2
#define JERK 3

// inline is needed to avoid the "multiple definitions" error. Other option is to create the utils.cpp file, and put
// there the function (and here only the prototype)
inline float solvePolyOrder2(Eigen::Vector3f coeff)
{
  // std::cout << "solving\n" << coeff << std::endl;
  float a = coeff[0];
  float b = coeff[1];
  float c = coeff[2];
  float dis = b * b - 4 * a * c;
  if (dis >= 0)
  {
    float x1 = (-b - sqrt(dis)) / (2 * a);  // x1 will always be smaller than x2
    float x2 = (-b + sqrt(dis)) / (2 * a);

    if (x1 >= 0)
    {
      return x1;
    }
    if (x2 >= 0)
    {
      return x2;
    }
  }
  printf("No solution found to the equation\n");
  return std::numeric_limits<float>::max();
}

inline std_msgs::ColorRGBA color(int id)
{
  std_msgs::ColorRGBA red;
  red.r = 1;
  red.g = 0;
  red.b = 0;
  red.a = 1;
  std_msgs::ColorRGBA blue;
  blue.r = 0;
  blue.g = 0;
  blue.b = 1;
  blue.a = 1;
  std_msgs::ColorRGBA blue_light;
  blue_light.r = 0.5;
  blue_light.g = 0.7;
  blue_light.b = 1;
  blue_light.a = 1;
  std_msgs::ColorRGBA green;
  green.r = 0;
  green.g = 1;
  green.b = 0;
  green.a = 1;
  std_msgs::ColorRGBA yellow;
  yellow.r = 1;
  yellow.g = 1;
  yellow.b = 0;
  yellow.a = 1;
  std_msgs::ColorRGBA orange_trans;  // orange transparent
  orange_trans.r = 1;
  orange_trans.g = 0.5;
  orange_trans.b = 0;
  orange_trans.a = 0.7;
  switch (id)
  {
    case RED:
      return red;
      break;
    case BLUE:
      return blue;
      break;
    case BLUE_LIGHT:
      return blue_light;
      break;
    case GREEN:
      return green;
      break;
    case YELLOW:
      return yellow;
      break;
    case ORANGE_TRANS:
      return orange_trans;
      break;
    default:
      ROS_ERROR("COLOR NOT DEFINED");
  }
}

// coeff is from highest degree to lowest degree. Returns the smallest positive real solution. Returns -1 if a
// root is imaginary or if it's negative

inline geometry_msgs::Point pointOrigin()
{
  geometry_msgs::Point tmp;
  tmp.x = 0;
  tmp.y = 0;
  tmp.z = 0;
  return tmp;
}

inline geometry_msgs::Point eigen2point(Eigen::Vector3d vector)
{
  geometry_msgs::Point tmp;
  tmp.x = vector[0];
  tmp.y = vector[1];
  tmp.z = vector[2];
  return tmp;
}

inline geometry_msgs::Vector3 vectorNull()
{
  geometry_msgs::Vector3 tmp;
  tmp.x = 0;
  tmp.y = 0;
  tmp.z = 0;
}

template <typename T>
using vec_E = std::vector<T, Eigen::aligned_allocator<T>>;

template <int N>
using Vecf = Eigen::Matrix<decimal_t, N, 1>;

template <int N>
using vec_Vecf = vec_E<Vecf<N>>;

// Given a path expressed by a vector of 3D-vectors (points), it returns its intersection with a sphere of
// radius=r and center=center
inline Eigen::Vector3f getIntersectionWithSphere(vec_Vecf<3> path, double r, Eigen::Vector3d center)
{
  int index = -1;
  for (int i = 0; i < path.size(); i++)
  {
    float dist = (path[i] - center).norm();
    if (dist > r)
    {
      index = i;  // This is the first point outside the sphere
      break;
    }
  }

  Eigen::Vector3d A;
  Eigen::Vector3d B;

  switch (index)
  {
    case -1:  // no points are outside the sphere --> find projection center-lastPoint into the sphere
      A = center;
      B = path[path.size() - 1];
      break;
    case 0:  // First element is outside the sphere
      printf("First element is oustide the sphere, there is sth wrong");
      break;
    default:
      A = path[index - 1];
      B = path[index];
  }

  // http://www.ambrsoft.com/TrigoCalc/Sphere/SpherLineIntersection_.htm

  std::cout << "Center=" << std::endl << center << std::endl;
  std::cout << "Radius=" << std::endl << r << std::endl;
  std::cout << "First Point=" << std::endl << A << std::endl;
  std::cout << "Second Point=" << std::endl << B << std::endl;

  float x1 = A[0];
  float y1 = A[1];
  float z1 = A[2];

  float x2 = B[0];
  float y2 = B[1];
  float z2 = B[2];

  float x3 = center[0];
  float y3 = center[1];
  float z3 = center[2];

  float a = pow((x2 - x1), 2) + pow((y2 - y1), 2) + pow((z2 - z1), 2);
  float b = 2 * ((x2 - x1) * (x1 - x3) + (y2 - y1) * (y1 - y3) + (z2 - z1) * (z1 - z3));
  float c = x3 * x3 + y3 * y3 + z3 * z3 + x1 * x1 + y1 * y1 + z1 * z1 - 2 * (x3 * x1 + y3 * y1 + z3 * z1) - r * r;

  float discrim = b * b - 4 * a * c;
  if (discrim <= 0)
  {
    printf("The line is tangent or doesn't intersect");
  }
  else
  {
    float t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    float x_int = x1 + (x2 - x1) * t;
    float y_int = y1 + (y2 - y1) * t;
    float z_int = z1 + (z2 - z1) * t;
    Eigen::Vector3f Intersection(x_int, y_int, z_int);
    std::cout << "Intersection=" << std::endl << Intersection << std::endl;
    return Intersection;
  }
}

#endif