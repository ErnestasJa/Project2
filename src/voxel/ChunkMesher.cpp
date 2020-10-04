#include "voxel/ChunkMesher.h"
#include "voxel/VoxelSide.h"
#include <voxel/VoxelInc.h>
#include <stack>

namespace vox {

struct MaskNode {
  uint8_t frontFace : 1;
  uint8_t backFace : 1;
  uint8_t align : 6;
  uint8_t r, g, b;

  MaskNode() { frontFace = backFace = false; }
};

struct Rect {
  Rect(int _x, int _y, int _x2, int _y2) : x(_x), y(_y), x2(_x2), y2(_y2) {}

  ~Rect() = default;

  int x, y, x2, y2;
};

#define COLOR_EQ(C, N) (C[0] == N.r && C[1] == N.g && C[2] == N.b)

inline int lengthr(int x, int y, const Rect &r, MaskNode mask[32][32],
                   bool front, uint8_t color[3]) {
  int l = x;
  for (; l <= r.x2 && (front ? mask[y][l].frontFace : mask[y][l].backFace) &&
         COLOR_EQ(color, mask[y][l]);
       l++)
    ;
  return l - x;
}

inline int heightr(int x, int y, int l, const Rect &r, MaskNode mask[32][32],
                   bool front, uint8_t color[3]) {
  int h = y;
  for (; h <= r.y2 && lengthr(x, h, r, mask, front, color) >= l; h++)
    ;
  return h - y;
}

inline void clearArea(MaskNode mask[32][32], bool front, int si, int sj, int i2,
                      int j2) {
  for (int j = sj; j < j2; j++)
    for (int i = si; i < i2; i++)
      if (front)
        mask[j][i].frontFace = false;
      else
        mask[j][i].backFace = false;
}

inline bool vf_equals(const VoxNode &n1, const VoxNode &n2) {
  return n1.size == 1 && n1.size == n2.size;
}

ChunkMesher::ChunkMesher() {
}

VoxNode ChunkMesher::GetBuildNode(uint32_t x, uint32_t y, uint32_t z) {
  if (x > 31 || y > 31 || z > 31) {
    return VoxNode(0, 0);
  } else {
    return m_buildNodes[x][y][z];
  }
}

bool ChunkMesher::CheckBuildNode(uint32_t x, uint32_t y, uint32_t z) {
  if (x > 31 || y > 31 || z > 31) {
    return false;
  } else {
    return m_buildNodes[x][y][z].size == 1;
  }
}

uint8_t ChunkMesher::GetVisibleBuildNodeSides(uint32_t x, uint32_t y,
                                              uint32_t z) {
  uint8_t sides = 0;

  if (x == 0 || m_buildNodes[x - 1][y][z].size == 0)
    sides |= RIGHT;

  if (x == 31 || m_buildNodes[x + 1][y][z].size == 0)
    sides |= LEFT;

  if (y == 0 || m_buildNodes[x][y - 1][z].size == 0)
    sides |= BOTTOM;

  if (y == 31 || m_buildNodes[x][y + 1][z].size == 0)
    sides |= TOP;

  if (z == 0 || m_buildNodes[x][y][z - 1].size == 0)
    sides |= BACK;

  if (z == 31 || m_buildNodes[x][y][z + 1].size == 0)
    sides |= FRONT;

  return sides;
}

void ChunkMesher::SetBuildNode(const VoxNode &node) {
  uint32_t x, y, z;
  decodeMK(node.start & LOCAL_VOXEL_MASK, x, y, z);
  VoxNode &bn = m_buildNodes[x][y][z];
  bn.start = node.start;
  bn.size = node.size;
  bn.r = node.r;
  bn.g = node.g;
  bn.b = node.b;
}

void ChunkMesher::ClearBuildNodes() {
  for (int x = 0; x < 32; x++)
    for (int y = 0; y < 32; y++)
      for (int z = 0; z < 32; z++)
        m_buildNodes[x][y][z].size = 0;
}

inline void ChunkMesher::BuildSliceMask(uint32_t dim, uint32_t slice,
                                        MaskNode mask[32][32]) {
  switch (dim) {
  case 0: {
    for (int y = 0; y < 32; y++)
      for (int x = 0; x < 32; x++) {
        auto &mask_node = mask[y][x];

        auto cn = GetBuildNode(x, y, slice);
        mask_node.r = cn.r;
        mask_node.g = cn.g;
        mask_node.b = cn.b;

        if (cn.size == 1) {
          mask_node.frontFace = CheckBuildNode(x, y, slice + 1) == false;
          mask_node.backFace = CheckBuildNode(x, y, slice - 1) == false;
        }
      }
    break;
  }
  case 1: {
    for (int y = 0; y < 32; y++)
      for (int x = 0; x < 32; x++) {
        auto &mask_node = mask[y][x];

        auto cn = GetBuildNode(x, slice, y);
        mask_node.r = cn.r;
        mask_node.g = cn.g;
        mask_node.b = cn.b;

        if (cn.size == 1) {
          mask_node.frontFace = CheckBuildNode(x, slice + 1, y) == false;
          mask_node.backFace = CheckBuildNode(x, slice - 1, y) == false;
        }
      }
    break;
  }
  case 2: {
    for (int y = 0; y < 32; y++)
      for (int x = 0; x < 32; x++) {
        auto &mask_node = mask[y][x];

        auto cn = GetBuildNode(slice, x, y);
        mask_node.r = cn.r;
        mask_node.g = cn.g;
        mask_node.b = cn.b;

        if (cn.size == 1) {
          mask_node.frontFace = CheckBuildNode(slice + 1, x, y) == false;
          mask_node.backFace = CheckBuildNode(slice - 1, x, y) == false;
        }
      }
    break;
  }
  }
}

void ChunkMesher::BuildFacesFromMask(vox::VoxelMesh *mesh, int dim, int z,
                                     MaskNode mask[32][32], bool frontFace) {
  std::stack<Rect> scanArea;

  Rect full{0, 0, 31, 31};
  scanArea.push(full);

  int faceNumber = 1;

  uint8_t color[3];

  while (!scanArea.empty()) {
    Rect r = scanArea.top();
    scanArea.pop();

    for (int j = r.y; j <= r.y2; j++) {
      for (int i = r.x; i <= r.x2; i++) {

        if (frontFace ? mask[j][i].frontFace : mask[j][i].backFace) {
          color[0] = mask[j][i].r;
          color[1] = mask[j][i].g;
          color[2] = mask[j][i].b;

          int l = lengthr(i, j, r, mask, frontFace, color);
          int h = heightr(i, j + 1, l, full, mask, frontFace, color) + 1;

          int sx = r.x, sy = j + h, ex = r.x2, ey = r.y2; /// bot one
          if (sx <= ex && sy <= ey)
            scanArea.emplace(sx, sy, ex, ey);

          sx = r.x, sy = j + 1, ex = i - 1, ey = j + h - 1; /// left one
          if (sx <= ex && sy <= ey)
            scanArea.emplace(sx, sy, ex, ey);

          sx = i + l, sy = j, ex = r.x2, ey = j + h - 1; /// right one
          if (sx <= ex && sy <= ey)
            scanArea.emplace(sx, sy, ex, ey);

          AddFaceToMesh(mesh, frontFace, (FacePlane)dim, z, glm::ivec2(i, j),
                        glm::ivec2(l, h), color);
          clearArea(mask, frontFace, i, j, i + l, j + h);
          faceNumber++;

          goto out; // evil within
        }
      }
    }
  out:;
  }
}

void ChunkMesher::AddFaceToMesh(vox::VoxelMesh *mesh, bool frontFace,
                                FacePlane dir, uint32_t slice, glm::ivec2 start,
                                glm::ivec2 dims,
                                uint8_t color[3]) {
  glm::vec3 face[4];

  switch (dir) {
  case FacePlane::XY: // xy
  {
    face[0] = glm::vec3(start.x + dims.x, start.y + dims.y, slice + frontFace);
    face[1] = glm::vec3(start.x, start.y + dims.y, slice + frontFace);
    face[2] = glm::vec3(start.x, start.y, slice + frontFace);
    face[3] = glm::vec3(start.x + dims.x, start.y, slice + frontFace);

    break;
  }
  case FacePlane::XZ: // xz
  {
    face[0] = glm::vec3(start.x, slice + frontFace, start.y) ;
    face[1] = glm::vec3(start.x + dims.x, slice + frontFace, start.y);
    face[2] = glm::vec3(start.x + dims.x, slice + frontFace, start.y + dims.y);
    face[3] = glm::vec3(start.x, slice + frontFace, start.y + dims.y);

    break;
  }
  case FacePlane::YZ: // yz
  {
    face[0] = glm::vec3(slice + frontFace, start.x + dims.x, start.y);
    face[1] = glm::vec3(slice + frontFace, start.x + dims.x, start.y + dims.y);
    face[2] = glm::vec3(slice + frontFace, start.x, start.y + dims.y);
    face[3] = glm::vec3(slice + frontFace, start.x, start.y);

    break;
  }
  default:
    break;
  }

  AddQuadToMesh(mesh, face, dims, frontFace, dir, color);
}

void ChunkMesher::AddQuadToMesh(vox::VoxelMesh *mesh, const glm::vec3 *face,
                                glm::ivec2 dims, bool frontFace,
                                FacePlane facePlane,
                                const uint8_t color[3]) noexcept {
  auto &ibo = mesh->Indices;
  auto &vbo = mesh->Vertices;
  auto &uvbo = mesh->UVs;
  auto &normals = mesh->Normals;

  uint32_t indicesStart = vbo.size();
  glm::vec3 col(((float)color[0]) / 255.0f, ((float)color[1]) / 255.0f,
                ((float)color[2]) / 255.0f);

  vbo.emplace_back(face[0]);
  vbo.emplace_back(face[1]);
  vbo.emplace_back(face[2]);
  vbo.emplace_back(face[3]);

  float texId;

  if (facePlane == FacePlane::XZ) {
    if (frontFace)
      texId = color[0];
    else
      texId = color[2];
  } else {
    texId = color[1];
  }

  if (facePlane == FacePlane::YZ) {
    util::swap(dims.x, dims.y);
  }

  uvbo.emplace_back(glm::vec3{0.f, 0.f, texId});
  uvbo.emplace_back(glm::vec3{1.f * dims.x, 0.f, texId});
  uvbo.emplace_back(glm::vec3{1.f * dims.x, 1.f * dims.y, texId});
  uvbo.emplace_back(glm::vec3{0.f, 1.f * dims.y, texId});

  if (facePlane == FacePlane::XZ) {
    frontFace = !frontFace;
  }

  if (frontFace) {
    ibo.emplace_back(indicesStart);
    ibo.emplace_back(indicesStart + 2);
    ibo.emplace_back(indicesStart + 3);

    ibo.emplace_back(indicesStart);
    ibo.emplace_back(indicesStart + 1);
    ibo.emplace_back(indicesStart + 2);
  } else {
    ibo.emplace_back(indicesStart + 3);
    ibo.emplace_back(indicesStart + 2);
    ibo.emplace_back(indicesStart);

    ibo.emplace_back(indicesStart + 2);
    ibo.emplace_back(indicesStart + 1);
    ibo.emplace_back(indicesStart);
  }

  glm::vec3 normal;

  if(facePlane == FacePlane::XZ){
    normal = frontFace ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
  }
  else if(facePlane == FacePlane::XY){
    normal = frontFace ? glm::vec3(0, 0, 1) : glm::vec3(0, 0, -1) ;
  }
  else{
    normal = frontFace ? glm::vec3(1, 0, 0) : glm::vec3(-1, 0, 0) ;
  }

  normals.emplace_back(normal);
  normals.emplace_back(normal);
  normals.emplace_back(normal);
  normals.emplace_back(normal);
}

void ChunkMesher::GreedyBuildChunk(vox::VoxelMesh *mesh) {
  MaskNode mask[32][32];

  for (int dim = 0; dim < 3; dim++) {
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 32; j++) {
        mask[i][j].frontFace = false;
        mask[i][j].backFace = false;
      }
    }

    for (int z = 0; z < 32; z++) {
      BuildSliceMask(dim, z, mask);
      BuildFacesFromMask(mesh, dim, z, mask, true);
      BuildFacesFromMask(mesh, dim, z, mask, false);
    }
  }
}

void ChunkMesher::BuildChunk(core::Vector<VoxNode>::iterator begin, core::Vector<VoxNode>::iterator end, VoxelMesh* voxMesh) {
  if(begin == end){
    return;
  }

  ClearBuildNodes();

  uint32_t x, y, z;
  while(true){
    auto& chunkNode = *begin;
    decodeMK(chunkNode.start & LOCAL_VOXEL_MASK, x, y, z);

    ASSERT(x<32 && y<32 && z<32, core::string::format("node x=<{}>, y=<{}>, z=<{}>", x,y,z));

    auto & buildNode = m_buildNodes[x][y][z];
    buildNode.start = chunkNode.start & LOCAL_VOXEL_MASK;
    buildNode.r = chunkNode.r;
    buildNode.g = chunkNode.g;
    buildNode.b = chunkNode.b;
    buildNode.size = chunkNode.size;
    begin++;

    if(begin == end)
    {
      break;
    }
  }

  GreedyBuildChunk(voxMesh);
}

}