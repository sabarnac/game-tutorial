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

// https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
//   is a good read to understand the collision concepts here.

/**
 * A class for defining an axis-aligned bounding box (AABB) collider.
 */
class AxisAlignedBoundingBox
{
private:
  // The corner of the cube with the smallest coordinates.
  glm::vec3 minCorner;
  // The corner of the cube with the smallest coordinates.
  glm::vec3 maxCorner;

  // The list of the eight corners of the AABB.
  std::vector<glm::vec3> corners;

  /**
   * Calculate the corners of the AABB and save them in the vector.
   */
  void updateCorners()
  {
    // Grab the minimum and maximum values of the box at the x,y,z axes using the min/max-corners.
    glm::vec2 minMaxX(glm::min(minCorner.x, maxCorner.x), glm::max(minCorner.x, maxCorner.x));
    glm::vec2 minMaxY(glm::min(minCorner.y, maxCorner.y), glm::max(minCorner.y, maxCorner.y));
    glm::vec2 minMaxZ(glm::min(minCorner.z, maxCorner.z), glm::max(minCorner.z, maxCorner.z));

    // Iterate through the minimum and maximum coordinate values of all the axes
    for (auto x = 0; x < 2; x++)
    {
      for (auto y = 0; y < 2; y++)
      {
        for (auto z = 0; z < 2; z++)
        {
          // The combination of the various min/max values of each axis gives us a coordinate of the box.
          corners.push_back(glm::vec3(minMaxX[x], minMaxY[y], minMaxZ[z]));
        }
      }
    }
  }

  /**
   * Update the min/max-corners of the AABB using the list of vertices of the object provided.
   * 
   * @param vertices  The list of vertices of the object using which to calculate the min/max-corners.
   */
  void updateMinMaxCorners(std::vector<glm::vec3> vertices)
  {
    // Iterate through all the vertices
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      // For each axis of a vertex coordinate...
      for (auto axis = 0; axis < 3; axis++)
      {
        // Store that value if it is the lowest/highest value observed so far.
        minCorner[axis] = glm::min(minCorner[axis], (*vertex)[axis]);
        maxCorner[axis] = glm::max(maxCorner[axis], (*vertex)[axis]);
      }
    }
  }

public:
  AxisAlignedBoundingBox(glm::vec3 minCorner, glm::vec3 maxCorner)
      : minCorner(minCorner),
        maxCorner(maxCorner)
  {
    // Generate the corners of the AABB.
    updateCorners();
  }

  AxisAlignedBoundingBox(std::vector<glm::vec3> vertices)
      : minCorner(vertices[0]),
        maxCorner(vertices[0])
  {
    // Generate the min/max-corners of the AABB using the given vertices.
    updateMinMaxCorners(vertices);
    // Generate the corners of the AABB.
    updateCorners();
  }

  /**
   * Get the minimum size corner coordinates of the AABB.
   * 
   * @return The minimum size corner.
   */
  glm::vec3 &getMinCorner()
  {
    return minCorner;
  }

  /**
   * Get the maximum size corner coordinates of the AABB.
   * 
   * @return The maximum size corner.
   */
  glm::vec3 &getMaxCorner()
  {
    return maxCorner;
  }

  /**
   * Get all the eight corners of the AABB.
   * 
   * @return The list of the eight corners.
   */
  std::vector<glm::vec3> &getCorners()
  {
    return corners;
  }

  /**
   * Update the min/max-corners of the AABB using the given values.
   * Also updates the eight corners as well.
   * 
   * @param newMinCorner  The minimum size corner.
   * @param newMiaxorner  The maximum size corner.
   */
  void update(glm::vec3 newMinCorner, glm::vec3 newMaxCorner)
  {
    // Update the min-corner.
    minCorner = newMinCorner;
    // Update the max-corner.
    maxCorner = newMaxCorner;
    // Generate the corners of the AABB.
    updateCorners();
  }

  /**
   * Update the min/max-corners of the AABB using the provided set of vertices to calculate them.
   * Also updates the eight corners as well.
   * 
   * @param newVertices  The set of vertices to use to calculate the min/max-corners.
   */
  void update(std::vector<glm::vec3> newVertices)
  {
    // Set the min-corner to the first vertex in the list.
    minCorner = newVertices[0];
    // Set the max-corner to the first vertex in the list.
    maxCorner = newVertices[0];
    // Generate the min/max-corners of the AABB using the given vertices.
    updateMinMaxCorners(newVertices);
    // Generate the corners of the AABB.
    updateCorners();
  }

  /**
   * Calculate and return whether the current AABB has intersected/collided with the given AABB.
   * 
   * @param otherBox  The other AABB to check if the current AABB has collided with.
   * 
   * @return Whether a collision has occurred or not.
   */
  bool hasCollided(AxisAlignedBoundingBox &otherBox)
  {
    // Assume that a collision has occured.
    auto hasCollided = true;
    // Iterate through each axis of the min/max-corners of both AABBs.
    for (auto axis = 0; axis < 3; axis++)
    {
      // Check if a collision has occured on that axis and append to the result.
      hasCollided = hasCollided && (getMinCorner()[axis] <= otherBox.getMaxCorner()[axis] && getMaxCorner()[axis] >= otherBox.getMinCorner()[axis]);
    }
    // True is returned only if a collision has occured along all three axes, false otherwise.
    return hasCollided;
  }
};

/**
 * Enum of the supported collider shapes.
 */
enum ColliderShapeType
{
  SPHERE = 0,
  BOX = 1 // This box is NOT axis-aligned.
};

/**
 * A base class for supported collider shapes.
 */
class ColliderShape
{
protected:
  // The shape type of the collider.
  ColliderShapeType type;
  // The position of the collider.
  glm::vec3 position;
  // The rotation of the collider.
  glm::vec3 rotation;
  // The scale of the collider.
  glm::vec3 scale;

  // The base AABB collision box that is generated using the model prior to any transformations done to it.
  // This box as a result can perfectly move around with the model and transform along with it to align with it accordingly,
  //   allowing it to be used for deeper collision checks.
  // This may seem to defeat the purpose of the AABB since it looses its axis-aligned properties, but if we inverse-transform
  //   the entire world back using the models' transformation matrix, the base AABB becomes a proper AABB again, and everything
  //   is now aligned with that AABB, making it possible to detect collisions against it again.
  std::shared_ptr<AxisAlignedBoundingBox> baseBox;
  // Since the base AABB is being transformed around, another AABB is generated using the base AABB post-transformation.
  //   This gives us a base AABB to perform a shallow collision check against.
  std::shared_ptr<AxisAlignedBoundingBox> transformedBox;

  // Update the transformed AABB using the transformed base AABB.
  void updateTransformedBox()
  {
    // Get all the corners of the base AABB.
    auto baseBoxCorners = baseBox->getCorners();
    // Define a vector where we will store the transformed corners of the base AABB.
    std::vector<glm::vec3> newCorners({});
    // Iterate through each corner of the base AABB.
    for (auto corner = baseBoxCorners.begin(); corner != baseBoxCorners.end(); corner++)
    {
      // Transform the corner using the models' transformation matrix and store the result.
      newCorners.push_back(glm::vec3(glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale) * glm::vec4(*corner, 1.0)));
    }
    // Generate the new AABB using the transformed base AABB.
    transformedBox = std::make_shared<AxisAlignedBoundingBox>(newCorners);
  }

  void updateBaseBox(std::shared_ptr<AxisAlignedBoundingBox> newBaseBox)
  {
    // Update the base AABB.
    baseBox = newBaseBox;
    // Generate the transformation AABB.
    updateTransformedBox();
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
    // Generate the transformation AABB.
    updateTransformedBox();
  }

  virtual ~ColliderShape(){};

  /**
   * Returns the type of the collider shape.
   * 
   * @return The collider shape type.
   */
  ColliderShapeType getType()
  {
    return type;
  }

  /**
   * Returns the position of the collider.
   * 
   * @return The collider position.
   */
  glm::vec3 &getPosition()
  {
    return position;
  }

  /**
   * Returns the rotation of the collider.
   * 
   * @return The collider rotation.
   */
  glm::vec3 &getRotation()
  {
    return rotation;
  }

  /**
   * Returns the scale of the collider.
   * 
   * @return The collider scale.
   */
  glm::vec3 &getScale()
  {
    return scale;
  }

  /**
   * Returns the base AABB of the collider.
   * 
   * @return The collider base AABB.
   */
  std::shared_ptr<AxisAlignedBoundingBox> &getBaseBox()
  {
    return baseBox;
  }

  /**
   * Returns the transformation AABB of the collider.
   * 
   * @return The collider transformation AABB.
   */
  std::shared_ptr<AxisAlignedBoundingBox> &getTransformedBox()
  {
    return transformedBox;
  }

  /**
   * Update the transformations of the collider (position, rotation, scale).
   * 
   * @param newPosition  The new position of the collider.
   * @param newRotation  The new rotation of the collider.
   * @param newScale     The new scale of the collider.
   */
  virtual void updateTransformations(glm::vec3 newPosition, glm::vec3 newRotation, glm::vec3 newScale)
  {
    // Update the collider position.
    position = newPosition;
    // Update the collider rotation.
    rotation = newRotation;
    // Update the collider scale.
    scale = newScale;
    // Generate the transformation AABB.
    updateTransformedBox();
  }
};

/**
 * A collider that has the shape of a sphere.
 */
class SphereColliderShape : public ColliderShape
{
private:
  // The radius of the collider sphere.
  double radius;

  /**
   * Creates the colliders' base AABB using the radius of the sphere.
   * 
   * @param radius  The radius of the sphere.
   * 
   * @return The collider base AABB.
   */
  std::shared_ptr<AxisAlignedBoundingBox> createBaseBox(double radius)
  {
    // Generates the base AABB by using the negative and positive values of the radius to get the min/max-corners of the AABB.
    return std::make_shared<AxisAlignedBoundingBox>(glm::vec3(-radius, -radius, -radius), glm::vec3(radius, radius, radius));
  }

  /**
   * Calculates the radius of the collider sphere using the given vertices of the model.
   * 
   * @param vertices  The vertices of the model.
   * 
   * @return The radius of the sphere.
   */
  double createRadius(std::vector<glm::vec3> vertices)
  {
    // Set the farthest vertex distance from the centre of the model as the length of the vertex.
    // Since the position of the vertex is relative to the centre of the model, just calculating the length of the position
    //   vector is enough
    auto farthestVertexDistance = glm::length(vertices[0]);
    // Iterate through each vertex of the model.
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      // Calculate the distance of the vertex from the centre of the model.
      // Since the position of the vertex is relative to the centre of the model, just calculating the length of the position
      //   vector is enough
      auto currentVertexDistance = glm::length(*vertex);
      // Check if the current vector is further away than the last checked farthest vector.
      if (currentVertexDistance > farthestVertexDistance)
      {
        // If it is, save it as the new farthest distance.
        farthestVertexDistance = currentVertexDistance;
      }
    }
    // The largest distance of a vertex from the centre of the model can be used as the radius of the sphere. So return that as
    //   the result.
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

  /**
   * Returns the radius of the collider wphere.
   * 
   * @return The sphere radius.
   */
  double getRadius()
  {
    return radius;
  }

  /**
   * Update the transformations of the collider (position, rotation, scale).
   * 
   * @param newPosition  The new position of the collider.
   * @param newRotation  The new rotation of the collider.
   * @param newScale  The new scale of the collider.
   */
  void updateTransformations(glm::vec3 newPosition, glm::vec3 newRotation, glm::vec3 newScale) override
  {
    // Update the collider position.
    position = newPosition;
    // We're not updating the rotation because it is useless.
    // Rotating a sphere around the same point still gives the same sphere.
    // // rotation = newRotation;
    // Update the collider scale.
    scale = newScale;
    // Generate the transformation AABB.
    updateTransformedBox();
  }

  /**
   * Update the radius of the collider sphere.
   * 
   * @param newRadius  The new radius of the sphere.
   */
  void update(double newRadius)
  {
    // Update the collider sphere radius.
    radius = newRadius;
    // Generate the new base AABB of the collider.
    updateBaseBox(createBaseBox(newRadius));
  }

  /**
   * Update the radius of the collider sphere using the given list of vertices of the model.
   * 
   * @param newVertices  The list vertices of the model to use to calculate the new radius.
   */
  void update(std::vector<glm::vec3> newVertices)
  {
    // Update the collider sphere radius.
    radius = createRadius(newVertices);
    // Generate the new base AABB of the collider.
    updateBaseBox(createBaseBox(radius));
  }
};

/**
 * A collider that has the shape of a box (just like an AABB, but not aligned).
 */
class BoxColliderShape : public ColliderShape
{
private:
  // The list of the eight corners of the collider box.
  std::vector<glm::vec3> corners;

  /**
   * Creates the colliders' base AABB using the corners of the box.
   * 
   * @param vertices  The vertices of the box.
   * 
   * @return The collider base AABB.
   */
  std::shared_ptr<AxisAlignedBoundingBox> createBaseBox(std::vector<glm::vec3> corners)
  {
    // Just forward the corners of the box to the AABB constructor, which can generate the AABB accordingly.
    return std::make_shared<AxisAlignedBoundingBox>(corners);
  }

  /**
   * Calculates the corners of the collider box using the given vertices of the model.
   * 
   * @param vertices  The vertices of the model.
   * 
   * @return The corners of the box.
   */
  std::vector<glm::vec3> createCorners(std::vector<glm::vec3> vertices)
  {
    // Set the min-corner to the first vertex in the list.
    glm::vec3 minCorner = vertices[0];
    // Set the max-corner to the first vertex in the list.
    glm::vec3 maxCorner = vertices[0];

    // Iterate through all the vertices.
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++)
    {
      // For each axis of a vertex coordinate...
      for (auto axis = 0; axis < 3; axis++)
      {
        // Store that value if it is the lowest/highest value observed so far.
        minCorner[axis] = glm::min(minCorner[axis], (*vertex)[axis]);
        maxCorner[axis] = glm::max(maxCorner[axis], (*vertex)[axis]);
      }
    }

    // Grab the minimum and maximum values of the box at the x,y,z axes using the min/max-corners.
    glm::vec2 minMaxX(glm::min(minCorner.x, maxCorner.x), glm::max(minCorner.x, maxCorner.x));
    glm::vec2 minMaxY(glm::min(minCorner.y, maxCorner.y), glm::max(minCorner.y, maxCorner.y));
    glm::vec2 minMaxZ(glm::min(minCorner.z, maxCorner.z), glm::max(minCorner.z, maxCorner.z));

    // Define a vector for storing the corners of the collider box.
    std::vector<glm::vec3> newCorners({});
    // Iterate through the minimum and maximum coordinate values of all the axes
    for (auto x = 0; x < 2; x++)
    {
      for (auto y = 0; y < 2; y++)
      {
        for (auto z = 0; z < 2; z++)
        {
          // The combination of the various min/max values of each axis gives us a coordinate of the box.
          newCorners.push_back(glm::vec3(minMaxX[x], minMaxY[y], minMaxZ[z]));
        }
      }
    }

    // Return the corners of the box.
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

  /**
   * Get all the eight corners of the collider box.
   * 
   * @return The list of the eight corners.
   */
  std::vector<glm::vec3> &getCorners()
  {
    return corners;
  }

  /**
   * Update the corners of the collider boxusing the given opposite corners for the new box.
   * 
   * @param newOppositeCorner1  The first corner among the opposite corner pair.
   * @param newOppositeCorner2  The second corner among the opposite corner pair.
   */
  void update(glm::vec3 newOppositeCorner1, glm::vec3 newOppositeCorner2)
  {
    // Generate the new corners of the box using the opposite corners provided.
    corners = createCorners(std::vector<glm::vec3>({newOppositeCorner1, newOppositeCorner2}));
    // Generate the new base AABB of the collider.
    updateBaseBox(createBaseBox(corners));
  }

  /**
   * Update the corners of the collider box using the given list of vertices of the model.
   * 
   * @param newVertices  The list vertices of the model to use to calculate the new corners.
   */
  void update(std::vector<glm::vec3> newVertices)
  {
    // Generate the new corners of the box using the given vertices of the model.
    corners = createCorners(newVertices);
    // Generate the new base AABB of the collider.
    updateBaseBox(createBaseBox(corners));
  }
};

/**
 * A class that can perform a collision check between all supported collider types.
 */
class DeepCollisionValidator
{
private:
  /**
   * Check if two sphere colliders have interesected/collided with each other.
   * 
   * @param sphere1  The first sphere collider.
   * @param sphere2  The second sphere collider.
   * 
   * @return Whether the two sphere colliders have collided or not.
   */
  static bool haveSphereSphereCollided(std::shared_ptr<SphereColliderShape> sphere1, std::shared_ptr<SphereColliderShape> sphere2)
  {
    // Calculate the scaled radius of the first collider sphere based on the scale of the collider.
    auto sphere1ScaledRadius = sphere1->getRadius() * sphere1->getScale().x;
    // Calculate the scaled radius of the second collider sphere based on the scale of the collider.
    auto sphere2ScaledRadius = sphere2->getRadius() * sphere2->getScale().x;

    // Calculate the distance between the centres of the spheres.
    auto distanceBetweenSpheres = glm::distance(sphere1->getPosition(), sphere2->getPosition());
    // If the distance between the spheres is greater than the sum of the radii of the two spheres, then the two have not collided.
    return distanceBetweenSpheres <= (sphere1ScaledRadius + sphere2ScaledRadius);
  }

  /**
   * Check if a box collider and a spehere collider have interesected/collided with each other.
   * 
   * @param box     The box collider.
   * @param sphere  The sphere collider.
   * 
   * @return Whether the box and sphere colliders have collided or not.
   */
  static bool haveBoxSphereCollided(std::shared_ptr<BoxColliderShape> box, std::shared_ptr<SphereColliderShape> sphere)
  {
    // Get the transformation matrix of the collider box.
    auto boxTransformationMatrix = glm::translate(box->getPosition()) * glm::toMat4(glm::quat(box->getRotation())) * glm::scale(box->getScale()) * glm::mat4();
    // Calculate the inverse of the boxes' transformation matrix.
    auto boxInverseTransformationMatrix = glm::inverse(boxTransformationMatrix);
    // Create an AABB using the corners of the box (doing this just as a way to get the min/max-corners).
    AxisAlignedBoundingBox boxAABB(box->getCorners());
    // Get the min-corner of the box.
    auto boxAABBMinCorners = boxAABB.getMinCorner();
    // Get the max-corner of the box.
    auto boxAABBMaxCorners = boxAABB.getMaxCorner();

    // Calculate the scaled radius of the collider sphere based on the scale of the collider.
    auto sphereScaledRadius = sphere->getRadius() * sphere->getScale().x;
    // Calculate the position of the wphere w.r.t the box using the boxes' inverse transformation matrix.
    auto spherePositionInBoxSpace = glm::vec3(boxInverseTransformationMatrix * glm::vec4(sphere->getPosition(), 1.0));
    // Calculate the point on the box that is closest to the sphere.
    auto boxPointClosesToSphere = glm::vec3(
        glm::max(boxAABB.getMinCorner().x, glm::min(spherePositionInBoxSpace.x, boxAABB.getMaxCorner().x)),
        glm::max(boxAABB.getMinCorner().y, glm::min(spherePositionInBoxSpace.y, boxAABB.getMaxCorner().y)),
        glm::max(boxAABB.getMinCorner().z, glm::min(spherePositionInBoxSpace.z, boxAABB.getMaxCorner().z)));
    // Calculate the distance between the boxes' closest point and the centre of the sphere.
    auto distanceBetweenClosestBoxPointAndSphereCenter = glm::distance(boxPointClosesToSphere, spherePositionInBoxSpace);

    // If the distance between the closest point on the box and the centre of the sphere is greater than the radius of the sphere,
    //   then the two have not collided.
    return distanceBetweenClosestBoxPointAndSphereCenter <= sphereScaledRadius;
  }

  /**
   * Check if two box colliders have interesected/collided with each other.
   * 
   * @param box1  The first box collider.
   * @param box2  The second box collider.
   * 
   * @return Whether the two box colliders have collided or not.
   */
  static bool haveBoxBoxCollided(std::shared_ptr<BoxColliderShape> box1, std::shared_ptr<BoxColliderShape> box2)
  {
    // Get the transformation matrix of the first collider box.
    auto box1TransformationMatrix = glm::translate(box1->getPosition()) * glm::toMat4(glm::quat(box1->getRotation())) * glm::scale(box1->getScale()) * glm::mat4();
    // Calculate the inverse of the first boxes' transformation matrix.
    auto box1InverseTransformationMatrix = glm::inverse(box1TransformationMatrix);

    // Get the transformation matrix of the second collider box.
    auto box2TransformationMatrix = glm::translate(box2->getPosition()) * glm::toMat4(glm::quat(box2->getRotation())) * glm::scale(box2->getScale()) * glm::mat4();
    // Calculate the inverse of the second boxes' transformation matrix.
    auto box2InverseTransformationMatrix = glm::inverse(box2TransformationMatrix);

    // Get the corners of the first box.
    auto box1Corners = box1->getCorners();
    // Define a vector for storing the transformed corners of the first box.
    std::vector<glm::vec3> box1TransformedCorners({});
    // Iterate through the corners of the first box.
    for (auto box1Corner = box1Corners.begin(); box1Corner != box1Corners.end(); box1Corner++)
    {
      // Transform the corner using the boxes' transformation matrix and add it to the result list.
      box1TransformedCorners.push_back(glm::vec3(box1TransformationMatrix * glm::vec4(*box1Corner, 1.0)));
    }

    auto box2Corners = box2->getCorners();
    // Define a vector for storing the transformed corners of the first box.
    std::vector<glm::vec3> box2TransformedCorners({});
    // Iterate through the corners of the second box.
    for (auto box2Corner = box2Corners.begin(); box2Corner != box2Corners.end(); box2Corner++)
    {
      // Transform the corner using the boxes' transformation matrix and add it to the result list.
      box2TransformedCorners.push_back(glm::vec3(box2TransformationMatrix * glm::vec4(*box2Corner, 1.0)));
    }

    // Create an AABB using the corners of the second box (doing this just as a way to get the min/max-corners).
    AxisAlignedBoundingBox box2AABB(box2->getCorners());
    // Get the min-corner of the second box.
    auto box2AABBMinCorners = box2AABB.getMinCorner();
    // Get the max-corner of the second box.
    auto box2AABBMaxCorners = box2AABB.getMaxCorner();
    // Iterate through the transformed vertices of the first box.
    for (auto box1Corner = box1TransformedCorners.begin(); box1Corner != box1TransformedCorners.end(); box1Corner++)
    {
      // Transform the vertex of the first box into the space of the second box
      auto box1CornerInBox2Space = glm::vec3(box2InverseTransformationMatrix * glm::vec4(*box1Corner, 1.0));
      // Check if the vertex of the first box has collided with the second box.
      auto isBox1CornerInBox2 = (box1CornerInBox2Space.x >= box2AABBMinCorners.x && box1CornerInBox2Space.x <= box2AABBMaxCorners.x) &&
                                (box1CornerInBox2Space.y >= box2AABBMinCorners.y && box1CornerInBox2Space.y <= box2AABBMaxCorners.y) &&
                                (box1CornerInBox2Space.z >= box2AABBMinCorners.z && box1CornerInBox2Space.z <= box2AABBMaxCorners.z);
      // If we have detected a collision, then the two boxes have collided.
      if (isBox1CornerInBox2)
      {
        return true;
      }
    }

    // Create an AABB using the corners of the first box (doing this just as a way to get the min/max-corners).
    AxisAlignedBoundingBox box1AABB(box1->getCorners());
    // Get the min-corner of the first box.
    auto box1AABBMinCorners = box1AABB.getMinCorner();
    // Get the max-corner of the first box.
    auto box1AABBMaxCorners = box1AABB.getMaxCorner();
    // Iterate through the transformed vertices of the second box.
    for (auto box2Corner = box2TransformedCorners.begin(); box2Corner != box2TransformedCorners.end(); box2Corner++)
    {
      // Transform the vertex of the second box into the space of the first box
      auto box2CornerInBox1Space = glm::vec3(box1InverseTransformationMatrix * glm::vec4(*box2Corner, 1.0));
      // Check if the vertex of the second box has collided with the first box.
      auto isBox2CornerInBox1 = (box2CornerInBox1Space.x >= box1AABBMinCorners.x && box2CornerInBox1Space.x <= box1AABBMaxCorners.x) &&
                                (box2CornerInBox1Space.y >= box1AABBMinCorners.y && box2CornerInBox1Space.y <= box1AABBMaxCorners.y) &&
                                (box2CornerInBox1Space.z >= box1AABBMinCorners.z && box2CornerInBox1Space.z <= box1AABBMaxCorners.z);
      // If we have detected a collision, then the two boxes have collided.
      if (isBox2CornerInBox1)
      {
        return true;
      }
    }

    // No collision has been detected, so return false.
    return false;
  }

public:
  /**
   * Checks if the two given collider shapes have intersected/collided with each other.
   * 
   * @param shape1              The first collider shape.
   * @param shape2              The second collider shape.
   * @param deepCollisionCheck  Whether to perform a deep collision check or not. If not, we just check if the
   *                              AABBs of the two colliders have collided or not.
   * 
   * @return Whether the two colldier shapes have collided or not.
   */
  static bool haveShapesCollided(std::shared_ptr<ColliderShape> shape1, std::shared_ptr<ColliderShape> shape2, bool deepCollisionCheck)
  {
    // Check if the AABBs of the two shapes have collided or not.
    if (!shape1->getTransformedBox()->hasCollided(*(shape2->getTransformedBox())))
    {
      // If not, no need to do a deeper check, so just return false.
      return false;
    }
    // If a deep collision check wasn't request, then just return true since the shallow check has passed.
    if (!deepCollisionCheck)
    {
      return true;
    }

    // Calculate a mask to determine what shape collision function is required.
    auto shapeTypeMask = (shape1->getType()) + (2 * shape2->getType());
    // Put the mask through a switch case.
    switch (shapeTypeMask)
    {
    case 0:
      // Both colliders are a sphere. Perform the check accordingly.
      return haveSphereSphereCollided(std::dynamic_pointer_cast<SphereColliderShape>(shape1), std::dynamic_pointer_cast<SphereColliderShape>(shape2));
    case 1:
      // First collider is a box, second is a sphere. Perform the check accordingly.
      return haveBoxSphereCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape1), std::dynamic_pointer_cast<SphereColliderShape>(shape2));
    case 2:
      // First collider is a sphere, second is a box. Perform the check accordingly.
      return haveBoxSphereCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape2), std::dynamic_pointer_cast<SphereColliderShape>(shape1));
    case 3:
      // Both colliders are a box. Perform the check accordingly.
      return haveBoxBoxCollided(std::dynamic_pointer_cast<BoxColliderShape>(shape1), std::dynamic_pointer_cast<BoxColliderShape>(shape2));
    default:
      // Can't determine the shapes, so say the two haven't collided.
      return false;
    }
  }
};

/**
 * Class for containing the details of the collider.
 */
class ColliderDetails
{
private:
  // The name of the collider.
  std::string colliderName;
  // The shape of the collider.
  std::shared_ptr<ColliderShape> colliderShape;

public:
  ColliderDetails(
      std::string colliderName,
      std::shared_ptr<ColliderShape> colliderShape)
      : colliderName(colliderName),
        colliderShape(colliderShape) {}

  /**
   * Get the name of the colldier.
   * 
   * @return The collider name.
   */
  std::string getColliderName()
  {
    return colliderName;
  }

  /**
   * Get the shape of the colldier.
   * 
   * @return The collider shape.
   */
  std::shared_ptr<ColliderShape> &getColliderShape()
  {
    return colliderShape;
  }
};

#endif