#pragma once

#include <x3d/Vector3.hpp>
#include <x3d/Matrix4.hpp>

class Camera {
public:
  Camera();

  void rotate_local(float angle, const x3d::Vector3f& axis);
  void rotate_global(float angle, const x3d::Vector3f& axis);
  void move_global(const x3d::Vector3f& rel);
  void move_local(const x3d::Vector3f& rel);

  x3d::Matrix4f apply();
protected:
  x3d::Vector3f m_position;
  x3d::Matrix4f m_rotation;
};