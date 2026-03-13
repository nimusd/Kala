#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>

/**
 * Spectrum - Array of harmonic amplitudes
 *
 * Used to pass harmonic content between containers.
 * Index 0 = fundamental, index 1 = 2nd harmonic, etc.
 */
class Spectrum
{
public:
    Spectrum(int numHarmonics = 64);

    void setAmplitude(int harmonicIndex, double amplitude);
    double getAmplitude(int harmonicIndex) const;

    int getNumHarmonics() const { return harmonics.size(); }
    void resize(int numHarmonics);
    void clear();

    // Direct access for efficiency
    const std::vector<double>& getHarmonics() const { return harmonics; }
    std::vector<double>& getHarmonics() { return harmonics; }

private:
    std::vector<double> harmonics;
};

#endif // SPECTRUM_H
