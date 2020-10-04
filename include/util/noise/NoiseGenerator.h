#ifndef THEPROJECT2_NOISEGENERATOR_H
#define THEPROJECT2_NOISEGENERATOR_H
#include "FastNoiseSIMD/FastNoiseSIMD.h"

namespace util::noise {
struct NoiseGeneratorSettings {

  NoiseGeneratorSettings()
      : Translation(0, 0, 0), FractalGain(.5f), FractalLacunarity(2.0f),
        Frequency(0.02f), FractalOctaves(5), Min(0), Max(1), Offset(0),
        Scale(64), Seed(12345) {}

  core::pod::Vec3<int32_t> Translation;
  float FractalGain;
  float FractalLacunarity;
  float Frequency;
  float Min;
  float Max;
  float Offset;
  float Scale;
  int32_t FractalOctaves;
  int32_t Seed;
};

class NoiseGenerator {
public:
  NoiseGenerator(core::pod::Vec3<int32_t> size) {
    m_size = size;

    m_fastNoise = core::UniquePtr<FastNoiseSIMD>(
        FastNoiseSIMD::NewFastNoiseSIMD(m_settings.Seed));
    m_noiseSet =
        core::UniquePtr<float[]>(new float[m_size.x * m_size.y * m_size.z]);
    SetNoiseGenSettings(m_settings);
  }

  void SetNoiseGenSettings(const NoiseGeneratorSettings &settings) {
    m_fastNoise->SetFractalGain(settings.FractalGain);
    m_fastNoise->SetFractalLacunarity(settings.FractalLacunarity);
    m_fastNoise->SetFrequency(settings.Frequency);
    m_fastNoise->SetFractalType(FastNoiseSIMD::FBM);
    m_fastNoise->SetFractalOctaves(settings.FractalOctaves);
    m_fastNoise->SetSeed(settings.Seed);

    m_settings = settings;
  }

  void Reseed(int seed) { m_fastNoise->SetSeed(seed); }

  void GenSimplex(float scaleFactor = 1.f) {
    m_fastNoise->FillSimplexFractalSet(
        m_noiseSet.get(), m_settings.Translation.x, m_settings.Translation.y,
        m_settings.Translation.z, m_size.x, m_size.y, m_size.z, scaleFactor);
  }

  float GetNoise(int index) {
    float value = ((m_noiseSet.get()[index] + 1) * 0.5f);
    value =
        glm::clamp(value, m_settings.Min, m_settings.Max) - m_settings.Offset;
    return value * m_settings.Scale;
  }

  float GetNoise(int32_t x, int32_t y, int32_t z) {
    int32_t index = z * m_size.x * m_size.y + y * m_size.x + x;
    float value = ((m_noiseSet.get()[index] + 1) * 0.5f);
    value =
        glm::clamp(value + m_settings.Offset, m_settings.Min, m_settings.Max);
    return value * m_settings.Scale;
  }

  [[nodiscard]] const NoiseGeneratorSettings &GetSettings() const { return m_settings; }

private:
  core::UniquePtr<FastNoiseSIMD> m_fastNoise;
  core::UniquePtr<float[]> m_noiseSet;
  core::pod::Vec3<int32_t> m_size;
  NoiseGeneratorSettings m_settings;
};

} // namespace util::noise

#endif // THEPROJECT2_NOISEGENERATOR_H
