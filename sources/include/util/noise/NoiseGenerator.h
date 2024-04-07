#ifndef THEPROJECT2_NOISEGENERATOR_H
#define THEPROJECT2_NOISEGENERATOR_H
#include "FastNoise/FastNoise.h"

namespace util::noise {
struct NoiseGeneratorSettings
{

  NoiseGeneratorSettings()
      : Translation(0, 0, 0)
      , FractalGain(.5f)
      , FractalLacunarity(2.0f)
      , Frequency(0.02f)
      , FractalOctaves(5)
      , Min(0)
      , Max(255)
      , Offset(0)
      , Scale(64)
      , Seed(12345)
  {
  }

  core::pod::Vec3<int32_t> Translation;
  float                    FractalGain;
  float                    FractalLacunarity;
  float                    Frequency;
  float                    Min;
  float                    Max;
  float                    Offset;
  float                    Scale;
  int32_t                  FractalOctaves;
  int32_t                  Seed;
};

class NoiseGenerator
{
  public:
  NoiseGenerator(core::pod::Vec3<int32_t> size)
  {
    m_size = size;

    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    m_fastNoise    = FastNoise::New<FastNoise::FractalFBm>();
    m_fastNoise->SetSource(fnSimplex);
    m_noiseSet = core::UniquePtr<float[]>(new float[m_size.x * m_size.y * m_size.z]);
    SetNoiseGenSettings(m_settings);
  }

  void SetNoiseGenSettings(const NoiseGeneratorSettings& settings)
  {
    ASSERT(m_fastNoise.get() != nullptr, "FastNoise not initialized");
    m_fastNoise->SetGain(settings.FractalGain);
    m_fastNoise->SetLacunarity(settings.FractalLacunarity);
    m_fastNoise->SetOctaveCount(settings.FractalOctaves);
    m_settings = settings;
  }

  void Reseed(int32_t seed)
  {
    m_settings.Seed = seed;
  }

  void GenSimplex()
  {
    ASSERT(m_fastNoise.get() != nullptr, "FastNoise not initialized");
    m_fastNoise->GenUniformGrid3D(m_noiseSet.get(), m_settings.Translation.x,
                                  m_settings.Translation.y, m_settings.Translation.z, m_size.x,
                                  m_size.y, m_size.z, m_settings.Frequency, m_settings.Seed);
  }

  float GetNoise(int32_t x, int32_t y, int32_t z)
  {
    int32_t index = z * m_size.x * m_size.y + y * m_size.x + x;
    ASSERT((x < m_size.x && y < m_size.y && z < m_size.z),
           core::string::format("m_size = [{}, {}, {}], [x, y, z] = [{}, {}, {}], index = {}",
                                m_size.x, m_size.y, m_size.z, x, y, z, index));
    float value = ((m_noiseSet.get()[index] + 1) * 0.5f);
    value       = glm::clamp(value + m_settings.Offset, m_settings.Min, m_settings.Max);
    return value * m_settings.Scale;
  }

  [[nodiscard]] const NoiseGeneratorSettings& GetSettings() const
  {
    return m_settings;
  }

  private:
  FastNoise::SmartNode<FastNoise::FractalFBm> m_fastNoise;
  core::UniquePtr<float[]>                    m_noiseSet;
  core::pod::Vec3<int32_t>                    m_size;
  NoiseGeneratorSettings                      m_settings;
};

} // namespace util::noise

#endif // THEPROJECT2_NOISEGENERATOR_H
