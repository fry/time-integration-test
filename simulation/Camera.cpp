#include "Camera.hpp"

Camera::Camera(): m_position(x3d::Vector3f::Zero) {
  m_rotation.set_identity();
}

void Camera::move_global(const x3d::Vector3f& rel) {
  m_position += rel;
}

void Camera::move_local(const x3d::Vector3f& rel) {
  x3d::Vector3f transformed_rel = m_rotation.transform_vector(rel);
  m_position += transformed_rel;
}

void Camera::rotate_local(float angle, const x3d::Vector3f& axis) {
  x3d::Matrix4f rot_mat; rot_mat.set_rotation(axis, angle);

  m_rotation *= rot_mat;

  x3d::Matrix4f mat_test = m_rotation;
  mat_test.translation() = m_position;
}

void Camera::rotate_global(float angle, const x3d::Vector3f& axis) {
  // Transform rotation axis to local coordinate system
  x3d::Vector3f local_axis = m_rotation.get_transpose().transform_vector(axis);
  rotate_local(angle, local_axis);
}

x3d::Matrix4f Camera::apply() {
  x3d::Matrix4f mat = m_rotation;

  // Invert z axis and rotation
  mat.z_axis() = -mat.z_axis();
  mat.transpose();

  // Append transformed inverse of position
  mat.translation() = mat.transform_vector(-m_position);

  return mat;
}
