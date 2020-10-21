#ifndef THEPROJECT2_INCLUDE_CORE_AXISALIGNEDBOUNDINGBOX_H_
#define THEPROJECT2_INCLUDE_CORE_AXISALIGNEDBOUNDINGBOX_H_

#include <glm/detail/type_vec3.hpp>
#include <glm/fwd.hpp>

namespace core {
class AxisAlignedBoundingBox {
public:
  AxisAlignedBoundingBox();
  AxisAlignedBoundingBox(const glm::vec3 &center, const glm::vec3 &halfsize);
  virtual ~AxisAlignedBoundingBox();

  void Reset(const glm::vec3 &point);
  void AddPoint(const glm::vec3 &point);
  core::Vector<glm::vec3> CalculatePoints() const;
  bool ContainsPoint(const glm::vec3 &point) const;
  bool IntersectsWith(const AxisAlignedBoundingBox &other) const;
  float SweepCollidesWith(const AxisAlignedBoundingBox &other, const glm::vec3 &vel,
                          glm::vec3 &normal) const;
  bool IntersectsWith(const glm::vec3 &aabbCenter,
                      const glm::vec3 &aabbHalfsize) const;
  bool CollidesWithRay(const glm::vec3 &rayStart,
                       const glm::vec3 &rayInverseDirection) const;
  void Translate(const glm::vec3 &point);
  void SetCenter(const glm::vec3 &point);

  glm::vec3 GetHalfSize() const;
  glm::vec3 GetCenter() const;
  glm::vec3 GetMin() const;
  glm::vec3 GetMax() const;

protected:
  glm::vec3 m_center, m_halfSize;
};
}

#endif // THEPROJECT2_INCLUDE_CORE_AXISALIGNEDBOUNDINGBOX_H_
