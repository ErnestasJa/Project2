#ifndef THEPROJECT2_INCLUDE_UTILS_BULKTEXTURELOADER_H_
#define THEPROJECT2_INCLUDE_UTILS_BULKTEXTURELOADER_H_

#include "filesystem/IFileSystem.h"
#include "resource_management/ImageLoader.h"
#include <sstream>

namespace utils {
std::string toUpper(std::string s){
  for(int i=0;i<(int)s.length();i++){s[i]=toupper(s[i]);}
  return s;
}
bool compareNat(const std::string& a, const std::string& b){
  if (a.empty())
    return true;
  if (b.empty())
    return false;
  if (std::isdigit(a[0]) && !std::isdigit(b[0]))
    return true;
  if (!std::isdigit(a[0]) && std::isdigit(b[0]))
    return false;
  if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
  {
    if (a[0] == b[0])
      return compareNat(a.substr(1), b.substr(1));
    return (toUpper(a) < toUpper(b));
    //toUpper() is a function to convert a std::string to uppercase.
  }

  // Both strings begin with digit --> parse both numbers
  std::istringstream issa(a);
  std::istringstream issb(b);
  int ia, ib;
  issa >> ia;
  issb >> ib;
  if (ia != ib)
    return ia < ib;

  // Numbers are the same --> remove numbers and recurse
  std::string anew, bnew;
  std::getline(issa, anew);
  std::getline(issb, bnew);
  return (compareNat(anew, bnew));
}

core::Vector<core::SharedPtr<render::ITexture>> LoadTextureAnimationFrames(io::IFileSystem* fs, res::ImageLoader * loader, core::String path){
  auto files = fs->GetFilesInDirectory(io::Path(path));

  core::Vector<core::SharedPtr<render::ITexture>> textures;

  elog::LogInfo("Files in directory:");

  std::sort(files.begin(), files.end(), [&](const io::Path & f1, const io::Path & f2){
    return compareNat(f1.AsString(), f2.AsString());
  });

  for(auto fileName: files){
    auto fn = fileName.AsString();
    auto ext = fileName.GetExtension().AsString();
    elog::LogInfo(core::string::CFormat("--| \"%s\", ext: \"%s\"", fn.c_str(), ext.c_str()));
  }
  elog::LogInfo("-------------------------------------");

  for(auto fileName: files){
    if(fileName.GetExtension().AsString() == ".png"){
      auto texture = loader->LoadImage(fileName);

      if(texture) {
        textures.push_back(texture);
        elog::LogInfo("Loaded texture: " + fileName.AsString());
      }
      else
      {
        elog::LogError("Failed to load texture: " + fileName.AsString());
      }
    }
  }
  elog::LogInfo(core::string::CFormat("Loaded %i textures",  textures.size()));
  return textures;
}

}

#endif // THEPROJECT2_INCLUDE_UTILS_BULKTEXTURELOADER_H_
