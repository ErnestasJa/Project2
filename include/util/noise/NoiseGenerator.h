#ifndef THEPROJECT2_NOISEGENERATOR_H
#define THEPROJECT2_NOISEGENERATOR_H
#include "FastNoiseSIMD/FastNoiseSIMD.h"

namespace util::noise {
struct NoiseGeneratorSettings {

  NoiseGeneratorSettings()
  : FractalGain(.5f),
    FractalLacunarity(2.0f),
    Frequency(0.02f),
    FractalOctaves(5),
    Min(0),
    Max(1),
    Offset(0),
    Scale(256)
  {
  }

  float FractalGain;
  float FractalLacunarity;
  float Frequency;
  float Min;
  float Max;
  float Offset;
  float Scale;
  int FractalOctaves;
};

class NoiseGenerator {
public:

NoiseGenerator(core::pod::Vec3<int32_t> size, int32_t seed = 12345){
  m_size = size;
  m_fastNoise = core::UniquePtr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD(seed));
  m_noiseSet = core::UniquePtr<float[]>(new float[m_size.x*m_size.y*m_size.z]);
  m_fastNoise->SetFractalGain(0.5);
  m_fastNoise->SetFractalLacunarity(2.0);
  m_fastNoise->SetFrequency(0.02f);
  m_fastNoise->SetFractalType(FastNoiseSIMD::FBM);
  m_fastNoise->SetFractalOctaves(5);
}

void SetNoiseGenSettings(const NoiseGeneratorSettings & settings){
  m_settings = settings;
  m_fastNoise->SetFractalGain(settings.FractalGain);
  m_fastNoise->SetFractalLacunarity(settings.FractalLacunarity);
  m_fastNoise->SetFrequency(settings.Frequency);
  m_fastNoise->SetFractalType(FastNoiseSIMD::FBM);
  m_fastNoise->SetFractalOctaves(settings.FractalOctaves);
}

void Reseed(int seed){
  m_fastNoise->SetSeed(seed);
}

void GenSimplex(float scaleFactor = 1.f){
  m_fastNoise->FillSimplexFractalSet(m_noiseSet.get(), 0, 0,0, m_size.x, m_size.y, m_size.z, scaleFactor);
}

float GetNoise(int index){
  float value = ((m_noiseSet.get()[index]+1)*0.5f);
  value = glm::clamp(value, m_settings.Min, m_settings.Max) - m_settings.Offset;
  return value * m_settings.Scale;
}

float GetNoise(int32_t x, int32_t y, int32_t z){
  int32_t index = z * m_size.x * m_size.y + y * m_size.x + x;
  float value = ((m_noiseSet.get()[index]+1)*0.5f);
  value = glm::clamp(value + m_settings.Offset, m_settings.Min, m_settings.Max) ;
  return value * m_settings.Scale;
}

private:
  core::UniquePtr<FastNoiseSIMD> m_fastNoise;
  core::UniquePtr<float[]> m_noiseSet;
  core::pod::Vec3<int32_t> m_size;
  NoiseGeneratorSettings m_settings;
};

}

#endif // THEPROJECT2_NOISEGENERATOR_H
