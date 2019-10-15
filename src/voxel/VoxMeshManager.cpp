#include "voxel/VoxMeshManager.h"
#include "voxel/Morton.h"
#include "stdlib.h"
#include <render/BaseMesh.h>
#include <render/IRenderer.h>

void AddMNode(render::BaseMesh* mesh, const MNode & node)
{

    /*BufferObject<glm::vec3> *vbo = (BufferObject<glm::vec3> *) mesh->buffers[Mesh::POSITION];
    IndexBufferObject<uint32_t> * ibo = (IndexBufferObject<uint32_t> *) mesh->buffers[Mesh::INDICES];
    BufferObject<glm::vec3> *cbo = (BufferObject<glm::vec3> *) mesh->buffers[Mesh::COLOR];

    uint32_t indicesStart = vbo->data.size();
    glm::vec3 col(1.0f, 1.0f, 1.0f);*/

    /*

    ///buffers
    vbo->data.push_back(face[0]);
    cbo->data.push_back(col);
    ibo->data.push_back(indicesStart+2);
*/}

VoxMeshManager::VoxMeshManager(render::IRenderer * renderer, MortonOctTreePtr octree, uint32_t level)
: m_renderer(renderer), m_octree(octree), m_level(level)
{
    ClearBuildNodes();
}

VoxMeshManager::~VoxMeshManager()
{
}

std::unordered_map<uint32_t, core::SharedPtr<render::BaseMesh>> & VoxMeshManager::GetMeshes()
{
    return m_map;
}

void VoxMeshManager::ClearBuildNodes()
{
    for(int x = 0; x < 32; x++)
    for(int y = 0; y < 32; y++)
    for(int z = 0; z < 32; z++)
    m_buildNodes[x][y][z].size = 0;;
    
}

void VoxMeshManager::SetBuildNode(const MNode & node)
{
    uint32_t x,y,z;
    decodeMK(node.start&LOCAL_VOXEL_MASK, x, y, z);
    MNode & bn = m_buildNodes[x][y][z];
    bn.start = node.start;
    bn.size = 1;
    bn.r = node.r;
    bn.g = node.g;
    bn.b = node.b;
}

bool vf_equals(const MNode & n1, const MNode & n2)
{
    return n1.size == 1 && n1.size == n2.size;
}


struct MaskNode
{
  uint8_t frontFace:1;
  uint8_t backFace:1;
  uint8_t align:6;
  uint8_t r,g,b;

  MaskNode()
  {
    frontFace=backFace=false;
  }
};


#define clear_mask(mask) loop(i,32) loop(j,32) {mask[i][j].frontFace = mask[i][j].backFace = false;}

inline void clearArea(MaskNode mask[32][32], bool front, int si, int sj, int i2, int j2)
{
  for(int j = sj; j < j2; j++)	   
    for(int i = si; i < i2; i++)		     
      if(front) mask[j][i].frontFace = false;
  else mask[j][i].backFace = false;
}

inline void VoxMeshManager::BuildSliceMask(uint32_t dim, uint32_t slice, MaskNode mask[32][32])
{
    MNode tmpNode;

    switch(dim)
    {
        case 0:
        {
            for(int y = 0; y < 32; y++)
            for(int x = 0; x < 32; x++)
            {
                MaskNode & n = mask[y][x];

                GetBuildNode(tmpNode,x,y,slice);
                n.r = tmpNode.r;
                n.g = tmpNode.g;
                n.b = tmpNode.b;

                if(tmpNode.size==1)
                {
                    GetBuildNode(tmpNode,x,y,slice+1);
                    n .frontFace = (tmpNode.size==0);

                    GetBuildNode(tmpNode,x,y,slice-1);
                    n .backFace = (tmpNode.size==0);
                }
            }
            break;
        }
        case 1:
        {
            for(int y = 0; y < 32; y++)
            for(int x = 0; x < 32; x++)
            {
                MaskNode & n = mask[y][x];

                GetBuildNode(tmpNode,x,slice,y);
                n.r = tmpNode.r;
                n.g = tmpNode.g;
                n.b = tmpNode.b;

                if(tmpNode.size==1)
                {
                    GetBuildNode(tmpNode,x,slice+1,y);
                    n.frontFace = (tmpNode.size==0);

                    GetBuildNode(tmpNode,x,slice-1,y);
                    n.backFace = (tmpNode.size==0);
                }
            }
            break;
        }
        case 2:
        {
            for(int y = 0; y < 32; y++)
            for(int x = 0; x < 32; x++)
            {
                MaskNode & n = mask[y][x];

                GetBuildNode(tmpNode,slice,x,y);
                n.r = tmpNode.r;
                n.g = tmpNode.g;
                n.b = tmpNode.b;

                if(tmpNode.size==1)
                {
                    GetBuildNode(tmpNode,slice+1,x,y);
                    n.frontFace = (tmpNode.size==0);

                    GetBuildNode(tmpNode,slice-1,x,y);
                    n.backFace = (tmpNode.size==0);
                }
            }
            break;
        }
    }
}

#define TOTAL_HEIGHT 32
#define TOTAL_WIDTH  32

#include <stack>

struct Rect
{
    int x,y,x2,y2;
};

#define COLOR_EQ(C, N) (C[0]==N.r&&C[1]==N.g&&C[2]==N.b)

int lengthr(int x, int y, Rect & r, MaskNode mask[32][32], bool front, uint8_t color[3])
{
    int l = x;
    for( ;l <= r.x2 && (front?mask[y][l].frontFace:mask[y][l].backFace) && COLOR_EQ(color,mask[y][l]); l++);
        return l-x;
}


int heightr(int x, int y, int l, Rect & r, MaskNode mask[32][32], bool front, uint8_t color[3])
{
    int h = y;
    for(; h <= r.y2 && lengthr(x,h,r,mask,front,color) >= l; h++);
        return h-y;
}


void VoxMeshManager::BuildFacesFromMask(render::BaseMesh* mesh, int dim, int z, const glm::vec3 & offset, MaskNode mask[32][32], bool frontFace)
{
    std::stack<Rect> scanArea;

    Rect full{0,0,31,31};
    scanArea.push(full);

    int faceNumber=1;

    uint8_t color[3];

    while(!scanArea.empty())
    {
        Rect r = scanArea.top();
        scanArea.pop();

        for(int j = r.y; j <= r.y2; j++)
        {
            for(int i = r.x; i <= r.x2; i++)
            {
                 if(frontFace ? mask[j][i].frontFace : mask[j][i].backFace)
                 {
                    color[0] = mask[j][i].r;
                    color[1] = mask[j][i].g;
                    color[2] = mask[j][i].b;

                    int l = lengthr(i,j,r, mask, frontFace, color);
                    int h = heightr(i,j+1,l,full, mask, frontFace, color)+1;

                    int sx = r.x, sy = j+h, ex = r.x2, ey = r.y2;   ///bot one
                    if(sx<=ex&&sy<=ey)
                        scanArea.push(Rect{sx,sy,ex,ey});

                    sx = r.x, sy = j+1, ex = i-1, ey = j+h-1;   ///left one
                    if(sx<=ex&&sy<=ey)
                        scanArea.push(Rect{sx,sy,ex,ey});

                    sx = i+l, sy = j, ex = r.x2, ey = j+h-1; ///right one
                    if(sx<=ex&&sy<=ey)
                        scanArea.push(Rect{sx,sy,ex,ey});

                    AddFaceToMesh(mesh, frontFace, dim, z, glm::ivec2(i,j), glm::ivec2(l,h), offset, color);
                    clearArea(mask,frontFace,i,j,i+l,j+h);
                    faceNumber++;

                    goto out; // evil within
                }
            }
        }
        out:;
    }
}

void VoxMeshManager::GreedyBuildChunk(render::BaseMesh* mesh, const glm::vec3 & offset)
{
    MaskNode mask[32][32];

    for(int dim = 0; dim < 3; dim++)
    {
        for(int i = 0; i < 32; i++) {
          for (int j = 0; j < 32; j++) {
            mask[i][j].frontFace = false;
            mask[i][j].backFace = false;
          }
        }


        for(int z = 0; z < 32; z++)
        {
            BuildSliceMask(dim,z,mask);
            BuildFacesFromMask(mesh, dim, z, offset, mask, true);
            BuildFacesFromMask(mesh, dim, z, offset, mask, false);
        }
    }
}

void VoxMeshManager::AddFaceToMesh(render::BaseMesh* mesh, bool frontFace, uint32_t dir, uint32_t slice, glm::ivec2 start, glm::ivec2 dims, glm::vec3 offset, uint8_t color[3])
{
    glm::vec3 face[4];

    switch(dir)
    {
    case 0: //xy
    {
        if(frontFace)
        {
            face[0]=glm::vec3(start.x,          start.y,              slice+1)+offset;
            face[1]=glm::vec3(start.x+dims.x,   start.y,              slice+1)+offset;
            face[2]=glm::vec3(start.x+dims.x,   start.y+dims.y,       slice+1)+offset;
            face[3]=glm::vec3(start.x,          start.y+dims.y,       slice+1)+offset;
        }
        else
        {
            face[3]=glm::vec3(start.x,          start.y,             slice)+offset;
            face[2]=glm::vec3(start.x+dims.x,   start.y,             slice)+offset;
            face[1]=glm::vec3(start.x+dims.x,   start.y+dims.y,      slice)+offset;
            face[0]=glm::vec3(start.x,          start.y+dims.y,      slice)+offset;
        }
        break;
    }
    case 1: //xz
    {
        if(frontFace)
        {
            face[3]=glm::vec3(start.x,         slice+1,                start.y)+offset;
            face[2]=glm::vec3(start.x+dims.x,  slice+1,                start.y)+offset;
            face[1]=glm::vec3(start.x+dims.x,  slice+1,                start.y+dims.y)+offset;
            face[0]=glm::vec3(start.x,         slice+1,                start.y+dims.y)+offset;
        }
        else
        {
            face[0]=glm::vec3(start.x,         slice,                  start.y)+offset;
            face[1]=glm::vec3(start.x+dims.x,  slice,                  start.y)+offset;
            face[2]=glm::vec3(start.x+dims.x,  slice,                  start.y+dims.y)+offset;
            face[3]=glm::vec3(start.x,         slice,                  start.y+dims.y)+offset;
        }

        break;
    }
    case 2: //yz
    {
        if(frontFace)
        {
            face[0]=glm::vec3(slice+1, start.x,         start.y      )+offset;
            face[1]=glm::vec3(slice+1, start.x +dims.x, start.y      )+offset;
            face[2]=glm::vec3(slice+1, start.x +dims.x, start.y+dims.y)+offset;
            face[3]=glm::vec3(slice+1, start.x,         start.y+dims.y)+offset;
        }
        else
        {
            face[3]=glm::vec3(slice, start.x,         start.y      )+offset;
            face[2]=glm::vec3(slice, start.x +dims.x, start.y      )+offset;
            face[1]=glm::vec3(slice, start.x +dims.x, start.y+dims.y)+offset;
            face[0]=glm::vec3(slice, start.x,         start.y+dims.y)+offset;
        }

        break;
    }
    default:
    break;
    }

    AddQuadToMesh(mesh, face, color);
}

void VoxMeshManager::AddQuadToMesh(render::BaseMesh* mesh, const glm::vec3 * face, uint8_t color[3])
{
  auto &ibo = mesh->IndexBuffer;
  auto &vbo = mesh->VertexBuffer;
  auto &cbo = mesh->ColorBuffer;

  uint32_t indicesStart = vbo.size();
  glm::vec3 col(((float)color[0])/255.0f, ((float)color[1])/255.0f, ((float)color[2])/255.0f);

  vbo.push_back(face[0]);
  vbo.push_back(face[1]);
  vbo.push_back(face[2]);
  vbo.push_back(face[3]);

  cbo.push_back(col);
  cbo.push_back(col);
  cbo.push_back(col);
  cbo.push_back(col);

  ibo.push_back(indicesStart);
  ibo.push_back(indicesStart+2);
  ibo.push_back(indicesStart+3);

  ibo.push_back(indicesStart);
  ibo.push_back(indicesStart+1);
  ibo.push_back(indicesStart+2);
}

uint8_t VoxMeshManager::GetVisibleBuildNodeSides(uint32_t x, uint32_t y, uint32_t z)
{
    uint8_t sides = 0;

    if(x==0 || m_buildNodes[x-1][y][z].size == 0 )
        sides |= RIGHT;

    if(x==31 || m_buildNodes[x+1][y][z].size == 0 )
        sides |= LEFT;

    if(y==0 || m_buildNodes[x][y-1][z].size == 0 )
        sides |= BOTTOM;

    if(y==31 || m_buildNodes[x][y+1][z].size == 0 )
        sides |= TOP;

    if(z==0 || m_buildNodes[x][y][z-1].size == 0 )
        sides |= BACK;

    if(z==31 || m_buildNodes[x][y][z+1].size == 0 )
        sides |= FRONT;

    return sides;
}

void VoxMeshManager::GetBuildNode(MNode & n, uint32_t x, uint32_t y, uint32_t z)
{
    if(x>31 || y>31 || z>31)
    {
        n.size = 0;
        n.start = 0;
    }
    else
    {
        MNode & node = m_buildNodes[x][y][z];
        n.size=node.size;
        n.start=node.start;
        n.r = node.r;
        n.g = node.g;
        n.b = node.b;
    }
}

void VoxMeshManager::GenAllChunks()
{
    if(m_octree->GetChildNodes().size()==0)
        return;

    faceCount = 0;
    ClearBuildNodes();


    uint32_t nodeChunk, x, y, z;
    uint32_t currentChunkMortonKey = nodeChunk = m_octree->GetChildNodes()[0].start & CHUNK_MASK;

    for( auto it = m_octree->GetChildNodes().begin(); it != m_octree->GetChildNodes().end(); it++ )
    {
        MNode & node = (*it);
        nodeChunk = node.start & CHUNK_MASK; /// get chunk (size 32x32x32)

        if(nodeChunk!=currentChunkMortonKey) /// THIS PIECE OF CODE IS GARBAGE, PLS FIX
        {
            decodeMK(currentChunkMortonKey,x,y,z);
            auto mesh = CreateEmptyMesh();
            m_map[currentChunkMortonKey] = mesh;
            GreedyBuildChunk(mesh.get(), glm::vec3(x,y,z));
            ClearBuildNodes();

            currentChunkMortonKey = nodeChunk;
        }

        SetBuildNode(node);
    }


    decodeMK(nodeChunk,x,y,z);
    auto mesh = CreateEmptyMesh();
    GreedyBuildChunk(mesh.get(), glm::vec3(x,y,z));
    m_map[nodeChunk] = mesh;

    for( MapIterator it = m_map.begin(); it != m_map.end(); it++ )
        it->second->Upload();

    std::cout << "Added " << faceCount << " faces to mesh" << std::endl;
}

void VoxMeshManager::AddVoxelToMesh(render::BaseMesh* mesh, const MNode & node, uint8_t sides)
{
  auto &ibo = mesh->IndexBuffer;
  auto &vbo = mesh->VertexBuffer;
  auto &cbo = mesh->ColorBuffer;

    uint32_t x, y, z;
    decodeMK(node.start, x,y,z);

    uint32_t si = vbo.size();

    //vbo->data.reserve(vbo->data.size()+8);
  vbo.push_back(glm::vec3(x,  y,  z));
  vbo.push_back(glm::vec3(x+1,y,  z));
  vbo.push_back(glm::vec3(x+1,y,  z+1));
  vbo.push_back(glm::vec3(x,  y,  z+1));

  vbo.push_back(glm::vec3(x,  y+1,z));
  vbo.push_back(glm::vec3(x+1,y+1,z));
  vbo.push_back(glm::vec3(x+1,y+1,z+1));
  vbo.push_back(glm::vec3(x,  y+1,z+1));

    ///color data
    glm::vec3 color(((float)node.r)/255.0f, ((float)node.g)/255.0f, ((float)node.b)/255.0f);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);
    cbo.push_back(color);

    ///faces
    //ibo->data.reserve(ibo->data.size()+36);

    //back face
    if(util::CheckBit(sides,BACK))
    {
        ibo.push_back(si);
        ibo.push_back(si+4);
        ibo.push_back(si+1);

        ibo.push_back(si+1);
        ibo.push_back(si+4);
        ibo.push_back(si+5);
    }

    //left face
    if(util::CheckBit(sides,LEFT))
    {
        ibo.push_back(si+1);
        ibo.push_back(si+5);
        ibo.push_back(si+2);

        ibo.push_back(si+2);
        ibo.push_back(si+5);
        ibo.push_back(si+6);
    }

    //front face
    if(util::CheckBit(sides,FRONT))
    {
        ibo.push_back(si+2);
        ibo.push_back(si+6);
        ibo.push_back(si+3);

        ibo.push_back(si+3);
        ibo.push_back(si+6);
        ibo.push_back(si+7);
    }

    //right face
    if(util::CheckBit(sides,RIGHT))
    {
        ibo.push_back(si+3);
        ibo.push_back(si+7);
        ibo.push_back(si+0);

        ibo.push_back(si+0);
        ibo.push_back(si+7);
        ibo.push_back(si+4);
    }

    //bot face
    if(util::CheckBit(sides,BOTTOM))
    {
        ibo.push_back(si+0);
        ibo.push_back(si+2);
        ibo.push_back(si+3);

        ibo.push_back(si+0);
        ibo.push_back(si+1);
        ibo.push_back(si+2);
    }

    //top face
    if(util::CheckBit(sides,TOP))
    {
        ibo.push_back(si+6);
        ibo.push_back(si+4);
        ibo.push_back(si+7);

        ibo.push_back(si+5);
        ibo.push_back(si+4);
        ibo.push_back(si+6);
    }
}

core::SharedPtr<render::BaseMesh> VoxMeshManager::CreateEmptyMesh()
{
    return m_renderer->CreateBaseMesh();
}

void VoxMeshManager::ClearMesh(render::BaseMesh * mesh)
{
    mesh->IndexBuffer.clear();
    mesh->VertexBuffer.clear();
    mesh->ColorBuffer.clear();
}

void VoxMeshManager::RebuildChunk(uint32_t chunk)
{
    uint32_t x,y,z;
    decodeMK(chunk,x,y,z);
    printf("Rebuilding chunk [%u,%u,%u]\n", x, y, z);
    auto & nodes = m_octree->GetChildNodes();
    auto it = std::lower_bound(nodes.begin(), nodes.end(), MNode(chunk));
    const uint32_t fchunk = (it->start&CHUNK_MASK);

    MapIterator mit;

    if(fchunk==chunk) /// chunk exists
        mit = m_map.find(chunk);
    else
    {
        m_map.erase(fchunk);
        return;
    }

    render::BaseMesh * mesh = nullptr;

    if(mit == m_map.end())
    {
        auto m = CreateEmptyMesh();
        m_map[chunk]=m;
        mesh = m.get();
    }
    else
    {
        mesh = mit->second.get();
        ClearMesh(mesh);
    }

    ClearBuildNodes();
    for( ; it != m_octree->GetChildNodes().end(); it++ )
    {
        MNode & node = (*it);
        const uint32_t nodeChunk = node.start & CHUNK_MASK; /// get chunk (size 32x32x32)

        if(nodeChunk!=fchunk)
            break;

        SetBuildNode( node );
    }

    GreedyBuildChunk(mesh,glm::vec3(x,y,z));

    mesh->Upload();
}

void VoxMeshManager::RenderAllMeshes()
{
    for( MapIterator it = m_map.begin(); it != m_map.end(); it++ )
        it->second->Render();
}