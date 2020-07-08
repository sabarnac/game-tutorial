#ifndef INCLUDE_COLLIDER_CPP
#define INCLUDE_COLLIDER_CPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>

#include <glm/glm.hpp>

class ColliderShape;
class SphereColliderShape;
class BoxColliderShape;

bool haveSphereSphereCollided(SphereColliderShape &sphere1, SphereColliderShape &sphere2);
bool haveBoxSphereCollided(BoxColliderShape &box, SphereColliderShape &sphere);
bool haveBoxBoxCollided(BoxColliderShape &box1, BoxColliderShape &box2);

/**
 * A class for defining an axis-aligned bounding box collider.
 */
class AxisAlignedBoundingBox
{
private:
  // The corner of the cube with the smallest coordinates.
  glm::vec3 minCorner;
  // The corner of the cube with the smallest coordinates.
  glm::vec3 maxCorner;

  std::vector<glm::vec3> corners;

  void updateCorners()
  {
    glm::vec2 minMaxX(glm::min(minCorner.x, maxCorner.x), glm::max(minCorner.x, maxCorner.x));
    glm::vec2 minMaxY(glm::min(minCorner.y, maxCorner.y), glm::max(minCorner.y, maxCorner.y));
    glm::vec2 minMaxZ(glm::min(minCorner.z, maxCorner.z), glm::max(minCorner.z, maxCorner.z));

    for (auto x = 0; x < 2; x++)
    {
      for (auto y = 0; y < 2; y++)
      {
        for (auto z = 0; z < 2; z++)
        {
          corners.push_back(glm::vec3(minMaxX[x], minMaxY[y], minMaxZ[z]));
        }
      }
    }
  }

public:
  AxisAlignedBoundingBox(glm::vec3 minCorner, glm::vec3 maxCorner)
      : minCorner(minCorner),
        maxCorner(maxCorner)
  {
    updateCorners();
  }

  AxisAlignedBoundingBox(std::vector<glm::vec3> vertices)
      : minCorner(vertices[0]),
        maxCorner(vertices[0])
  {
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      for (auto axis = 0; axis < 3; axis++)
      {
        minCorner[axis] = glm::min(minCorner[axis], (*vertex)[axis]);
        maxCorner[axis] = glm::max(maxCorner[axis], (*vertex)[axis]);
      }
    }
    updateCorners();
  }

  glm::vec3 &getMinCorner()
  {
    return minCorner;
  }

  glm::vec3 &getMaxCorner()
  {
    return maxCorner;
  }

  std::vector<glm::vec3> &getCorners()
  {
    return corners;
  }

  void update(glm::vec3 newMinCorner, glm::vec3 newMaxCorner)
  {
    minCorner = newMinCorner;
    maxCorner = newMaxCorner;
    updateCorners();
  }

  void update(std::vector<glm::vec3> newVertices)
  {
    minCorner = newVertices[0];
    maxCorner = newVertices[0];
    for (auto it = newVertices.begin(); it != newVertices.end(); it++)
    {
      for (auto axis = 0; axis < 3; axis++)
      {
        minCorner[axis] = glm::min(minCorner[axis], (*it)[axis]);
        maxCorner[axis] = glm::max(maxCorner[axis], (*it)[axis]);
      }
    }
    updateCorners();
  }

  bool hasCollided(AxisAlignedBoundingBox &otherBox)
  {
    auto hasCollided = true;
    for (auto axis = 0; axis < 3; axis++)
    {
      hasCollided = hasCollided && (getMinCorner()[axis] <= otherBox.getMaxCorner()[axis] && getMaxCorner()[axis] >= otherBox.getMinCorner()[axis]);
    }
    return hasCollided;
  }
};

enum ColliderShapeType
{
  SPHERE = 0,
  BOX = 1
};

class ColliderShape
{
protected:
  ColliderShapeType type;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  std::shared_ptr<AxisAlignedBoundingBox> baseBox;
  std::shared_ptr<AxisAlignedBoundingBox> transformedBox;

  void updateTransformedBox()
  {
    auto baseBoxCorners = baseBox->getCorners();
    std::vector<glm::vec3> newCorners({});
    for (auto corner = baseBoxCorners.begin(); corner != baseBoxCorners.end(); corner++)
    {
      newCorners.push_back(glm::vec3(glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale) * glm::vec4(*corner, 1.0)));
    }
    transformedBox = std::make_shared<AxisAlignedBoundingBox>(newCorners);
  }

public:
  ColliderShape(
      ColliderShapeType type,
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      std::shared_ptr<AxisAlignedBoundingBox> baseBox)
      : type(type),
        position(position),
        rotation(rotation),
        scale(scale),
        baseBox(baseBox)
  {
    updateTransformedBox();
  }

  virtual ~ColliderShape()
  {
    updateTransformedBox();
  }

  ColliderShapeType getType()
  {
    return type;
  }

  glm::vec3 &getPosition()
  {
    return position;
  }

  glm::vec3 &getRotation()
  {
    return rotation;
  }

  glm::vec3 &getScale()
  {
    return scale;
  }

  std::shared_ptr<AxisAlignedBoundingBox> &getBaseBox()
  {
    return baseBox;
  }

  std::shared_ptr<AxisAlignedBoundingBox> &getTransformedBox()
  {
    return transformedBox;
  }

  virtual void updateTransformations(glm::vec3 newPosition, glm::vec3 newRotation, glm::vec3 newScale)
  {
    position = newPosition;
    rotation = newRotation;
    scale = newScale;
    updateTransformedBox();
  }

  virtual void updateBaseBox(std::shared_ptr<AxisAlignedBoundingBox> newBaseBox)
  {
    baseBox = newBaseBox;
    updateTransformedBox();
  }
};

class SphereColliderShape : public ColliderShape
{
private:
  double radius;

  std::shared_ptr<AxisAlignedBoundingBox> createBaseBox(double radius)
  {
    return std::make_shared<AxisAlignedBoundingBox>(glm::vec3(-radius, -radius, -radius), glm::vec3(radius, radius, radius));
  }

  double createRadius(std::vector<glm::vec3> vertices)
  {
    auto farthestVertexDistance = glm::distance(vertices[0], glm::vec3(0.0));
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      auto currentVertexDistance = glm::distance(*vertex, glm::vec3(0.0));
      if (currentVertexDistance > farthestVertexDistance)
      {
        farthestVertexDistance = currentVertexDistance;
      }
    }
    return farthestVertexDistance;
  }

public:
  SphereColliderShape(
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      double radius)
      : ColliderShape(
            ColliderShapeType::SPHERE,
            position,
            rotation,
            scale,
            createBaseBox(radius)),
        radius(radius) {}

  SphereColliderShape(
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      std::vector<glm::vec3> vertices)
      : ColliderShape(
            ColliderShapeType::SPHERE,
            position,
            rotation,
            scale,
            createBaseBox(createRadius(vertices))),
        radius(createRadius(vertices)) {}

  double getRadius()
  {
    return radius;
  }

  void updateTransformations(glm::vec3 newPosition, glm::vec3 newRotation, glm::vec3 newScale) override
  {
    position = newPosition;
    scale = newScale;
    updateTransformedBox();
  }

  void update(double newRadius)
  {
    radius = newRadius;
    updateBaseBox(createBaseBox(newRadius));
  }

  void update(std::vector<glm::vec3> newVertices)
  {
    radius = createRadius(newVertices);
    updateBaseBox(createBaseBox(radius));
  }
};

class BoxColliderShape : public ColliderShape
{
private:
  std::vector<glm::vec3> corners;

  std::shared_ptr<AxisAlignedBoundingBox> createBaseBox(std::vector<glm::vec3> corners)
  {
    return std::make_shared<AxisAlignedBoundingBox>(corners);
  }

  std::vector<glm::vec3> createCorners(std::vector<glm::vec3> vertices)
  {
    glm::vec3 minCorner = vertices[0];
    glm::vec3 maxCorner = vertices[0];

    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      for (auto axis = 0; axis < 3; axis++)
      {
        minCorner[axis] = glm::min(minCorner[axis], (*vertex)[axis]);
        maxCorner[axis] = glm::max(maxCorner[axis], (*vertex)[axis]);
      }
    }

    glm::vec2 minMaxX(glm::min(minCorner.x, maxCorner.x), glm::max(minCorner.x, maxCorner.x));
    glm::vec2 minMaxY(glm::min(minCorner.y, maxCorner.y), glm::max(minCorner.y, maxCorner.y));
    glm::vec2 minMaxZ(glm::min(minCorner.z, maxCorner.z), glm::max(minCorner.z, maxCorner.z));

    std::vector<glm::vec3> newCorners({});
    for (auto x = 0; x < 2; x++)
    {
      for (auto y = 0; y < 2; y++)
      {
        for (auto z = 0; z < 2; z++)
        {
          newCorners.push_back(glm::vec3(minMaxX[x], minMaxY[y], minMaxZ[z]));
        }
      }
    }
    return newCorners;
  }

public:
  BoxColliderShape(
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      glm::vec3 oppositeCorner1,
      glm::vec3 oppositeCorner2)
      : ColliderShape(
            ColliderShapeType::BOX,
            position,
            rotation,
            scale,
            createBaseBox(createCorners(std::vector<glm::vec3>({oppositeCorner1, oppositeCorner2})))),
        corners(createCorners(std::vector<glm::vec3>({oppositeCorner1, oppositeCorner2}))) {}

  BoxColliderShape(
      glm::vec3 position,
      glm::vec3 rotation,
      glm::vec3 scale,
      std::vector<glm::vec3> vertices)
      : ColliderShape(
            ColliderShapeType::BOX,
            position,
            rotation,
            scale,
            createBaseBox(createCorners(vertices))),
        corners(createCorners(vertices)) {}

  std::vector<glm::vec3> &getCorners()
  {
    return corners;
  }

  void update(glm::vec3 newOppositeCorner1, glm::vec3 newOppositeCorner2)
  {
    corners = createCorners(std::vector<glm::vec3>({newOppositeCorner1, newOppositeCorner2}));
    updateBaseBox(createBaseBox(corners));
  }

  void update(std::vector<glm::vec3> newVertices)
  {
    corners = createCorners(newVertices);
    updateBaseBox(createBaseBox(corners));
  }
};

class DeepCollisionValidator
{
private:
  static bool haveSphereSphereCollided(std::shared_ptr<SphereColliderShape> sphere1, std::shared_ptr<SphereColliderShape> sphere2)
  {
    auto sphere1ScaledRadius = sphere1->getRadius() * sphere1->getScale().x;
    auto sphere2ScaledRadius = sphere2->getRadius() * sphere2->getScale().x;

    auto distanceBetweenSpheres = glm::distance(sphere1->getPosition(), sphere2->getPosition());
    return distanceBetweenSpheres <= (sphere1ScaledRadius + sphere2ScaledRadius);
  }

  static bool haveBoxSphereCollided(std::shared_ptr<BoxColliderShape> box, std::shared_ptr<SphereColliderShape> sphere)
  {
    auto boxTransformationMatrix = glm::translate(box->getPosition()) * glm::toMat4(glm::quat(box->getRotation())) * glm::scale(box->getScale()) * glm::mat4();
    auto boxInverseTransformationMatrix = glm::inverse(boxTransformationMatrix);
    AxisAlignedBoundingBox boxAABB(box->getCorners());
    auto boxAABBMinCorners = boxAABB.getMinCorner();
    auto boxAABBMaxCorners = boxAABB.getMaxCorner();

    auto sphereScaledRadius = sphere->getRadius() * sphere->getScale().x;
    auto spherePositionInBoxSpace = glm::vec3(boxInverseTransformationMatrix * glm::vec4(sphere->getPosition(), 1.0));
    auto boxPointClosesToSphere = glm::vec3(
        glm::max(boxAABB.getMinCorner().x, glm::min(spherePositionInBoxSpace.x, boxAABB.getMaxCorner().x)),
        glm::max(boxAABB.getMinCorner().y, glm::min(spherePositionInBoxSpace.y, boxAABB.getMaxCorner().y)),
        glm::max(boxAABB.getMinCorner().z, glm::min(spherePositionInBoxSpace.z, boxAABB.getMaxCorner().z)));
    auto distanceBetweenClosestBoxPointAndSphereCenter = glm::distance(boxPointClosesToSphere, spherePositionInBoxSpace);

    return distanceBetweenClosestBoxPointAndSphereCenter <= sphereScaledRadius;
  }

  static bool haveBoxBoxCollided(std::shared_ptr<BoxColliderShape> box1, std::shared_ptr<BoxColliderShape> box2)
  {
    auto box1TransformationMatrix = glm::translate(box1->getPosition()) * glm::toMat4(glm::quat(box1->getRotation())) * glm::scale(box1->getScale()) * glm::mat4();
    auto box2TransformationMatrix = glm::translate(box2->getPosition()) * glm::toMat4(glm::quat(box2->getRotation())) * glm::scale(box2->getScale()) * glm::mat4();

    auto box1Corners = box1->getCorners();
    std::vector<glm::vec3> box1TransformedCorners({});
    for (auto box1Corner = box1Corners.begin(); box1Corner != box1Corners.end(); box1Corner++)
    {
      box1TransformedCorners.push_back(glm::vec3(box1TransformationMatrix * glm::vec4(*box1Corner, 1.0)));
    }

    auto box2Corners = box2->getCorners();
    std::vector<glm::vec3> box2TransformedCorners({});
    for (auto box2Corner = box2Corners.begin(); box2Corner != box2Corners.end(); box2Corner++)
    {
      box2TransformedCorners.push_back(glm::vec3(box2TransformationMatrix * glm::vec4(*box2Corner, 1.0)));
    }

    auto box1InverseTransformationMatrix = glm::inverse(box1TransformationMatrix);
    auto box2InverseTransformationMatrix = glm::inverse(box2TransformationMatrix);

    AxisAlignedBoundingBox box2AABB(box2->getCorners());
    auto box2AABBMinCorners = box2AABB.getMinCorner();
    auto box2AABBMaxCorners = box2AABB.getMaxCorner();
    for (auto box1Corner = box1TransformedCorners.begin(); box1Corner != box1TransformedCorners.end(); box1Corner++)
    {
      auto box1CornerInBox2Space = glm::vec3(box2InverseTransformationMatrix * glm::vec4(*box1Corner, 1.0));
      auto isBox1CornerInBox2 = (box1CornerInBox2Space.x >= box2AABBMinCorners.x && box1CornerInBox2Space.x <= box2AABBMaxCorners.x) &&
                                (box1CornerInBox2Space.y >= box2AABBMinCorners.y && box1CornerInBox2Space.y <= box2AABBMaxCorners.y) &&
                                (box1CornerInBox2Space.z >= box2AABBMinCorners.z && box1CornerInBox2Space.z <= box2AABBMaxCorners.z);
      if (isBox1CornerInBox2)
      {
        return true;
      }
    }

    AxisAlignedBoundingBox box1AABB(box1->getCorners());
    auto box1AABBMinCorners = box1AABB.getMinCorner();
    auto box1AABBMaxCorners = box1AABB.getMaxCorner();
    for (auto box2Corner = box2TransformedCorners.begin(); box2Corner != box2TransformedCorners.end(); box2Corner++)
    {
      auto box2CornerInBox1Space = glm::vec3(box1InverseTransformationMatrix * glm::vec4(*box2Corner, 1.0));
      auto isBox2CornerInBox1 = (box2CornerInBox1Space.x >= box1AABBMinCorners.x && box2CornerInBox1Space.x <= box1AABBMaxCorners.x) &&
                                (box2CornerInBox1Space.y >= box1AABBMinCorners.y && box2CornerInBox1Space.y <= box1AABBMaxCorners.y) &&
                                (box2CornerInBox1Space.z >= box1AABBMinCorners.z && box2CornerInBox1Space.z <= box1AABBMaxCorners.z);
      if (isBox2CornerInBox1)
      {
        return true;
      }
    }

    return false;
  }

public:
  static bool haveShapesCollided(std::shared_ptr<ColliderShape> shape1, std::shared_ptr<ColliderShape> shape2, bool deepCollisionCheck)
  {
    if (!shape1->getTransformedBox()->hasCollided(*(shape2->getTransformedBox())))
    {
      return false;
    }
    if (!deepCollisionCheck)
    {
      return true;
    }

    auto shapeTypeMask = (shape1->getType()) + (2 * shape2->getType());
    switch (shapeTypeMask)
    {
    case 0:
      return haveSphereSphereCollided(std::dynamic_pointer_cast<SphereColliderShape>(shape1), std::dynamic_pointer_cast<SphereColliderShape>(shape2));
    case 1:
      return haveBoxSphereCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape1), std::dynamic_pointer_cast<SphereColliderShape>(shape2));
    case 2:
      return haveBoxSphereCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape2), std::dynamic_pointer_cast<SphereColliderShape>(shape1));
    case 3:
      return haveBoxBoxCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape1), std::dynamic_pointer_cast<BoxColliderShape>(shape2));
    default:
      return false;
    }
  }
};

class ColliderDetails
{
private:
  std::string colliderName;
  std::shared_ptr<ColliderShape> colliderShape;

public:
  ColliderDetails(
      std::string colliderName,
      std::shared_ptr<ColliderShape> colliderShape)
      : colliderName(colliderName),
        colliderShape(colliderShape) {}

  std::string getColliderName()
  {
    return colliderName;
  }

  std::shared_ptr<ColliderShape> &getColliderShape()
  {
    return colliderShape;
  }
};

#endif