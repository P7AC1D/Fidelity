#pragama once
#include <memory>
#include <vector>

#include "../Maths/Vector2.hpp"
#include "../Maths/Vector3.hpp"
#include "Types.h"

class IndexBuffer;
class VertexBuffer;

enum VertexDeclaration : ubyte
{
  Positions = 1 << 0,
  TextureCoords = 1 << 1,
  Normals = 1 << 2,
  Tangents = 1 << 3,
  Bitangets = 1 << 4,
};

class Material;

class Mesh
{
public:
  Mesh();
  
  void SetPositions(const std::vector<Vector3>& positions);
  void SetNormals(const std::vector<Vector3>& normals);
  void SetTangents(const std::vector<Vector3>& tangents);
  void SetBitangents(const std::vector<Vector3>& bitangets);
  void SetTextureCoords(const std::vector<Vector2>& textureCoords);
  
  void SetMaterial(const std::shared_ptr<Material> material);
  
  void Draw();
  
private:
  void UploadData();
  
private:
  std::vector<Vector3> _positions;
  std::vector<Vector3> _normals;
  std::vector<Vector3> _tangents;
  std::vector<Vector3> _bitangents;
  std::vector<Vector2> _textureCoords;
  
  uint32 _id;
  uint32 _materialId;
  ubyte _vertexDeclaration;
  bool _needsUpdate;
  
  std::unique_ptr<IndexBuffer> _indexBuffer;
  std::unique_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<Material> _material;
};
