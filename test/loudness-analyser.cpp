#include <loudnessIO/SoundFile.hpp>
#include <loudnessIO/ProcessFile.hpp>
#include <loudnessAnalyser/LoudnessAnalyser.hpp>

#include "gtest/gtest.h"

#include <string>
#include <cmath>
#include <stdlib.h>

// Value of EBU_LOUDNESS_TEST_SET_PATH environment variable
std::string ebuLoudnessTestSetPath;

/**
 * @brief Callback used while processing, to check progression value of the loudness analysis.
 */
void checkProgress(const int p)
{
    EXPECT_LE(p, 100);
}

/**
 * @brief Verifies that two specified doubles are equal, or within the specified accuracy of each other.
 * @param expected The first double to compare. This is the double the unit test expects.
 * @param actual The second double to compare. This is the double the unit test produced.
 * @param delta The required accuracy. The assertion will fail only if expected is different from actual by more than delta.
 */
void checkDoubleValue(const double expected, const double actual, const double delta = 0.05)
{
    // Currently, the gtest floating point comparison is too accurate for our results.
    // http://googletesting.blogspot.fr/2008/10/tott-floating-point-comparison.html
    // EXPECT_DOUBLE_EQ(actual, expected);
    EXPECT_TRUE(fabs(actual - expected) < delta);
}

/**
 * @brief Check the integrated Loudness value of the given input file.
 * @warn The file should be in a folder specified by EBU_LOUDNESS_TEST_SET_PATH environment variable.
 */
void checkEBUR128Analysis(
        const std::string filename,
        const double expectedIntegratedLoudnessValue,
        const double expectedMaxShortTerm,
        const double expectedMaxMomentary)
{
    const std::string absoluteFilename = ebuLoudnessTestSetPath + "/" + filename;
    Loudness::io::SoundFile audioFile;

    Loudness::analyser::LoudnessLevels level = Loudness::analyser::LoudnessLevels::Loudness_EBU_R128();
    Loudness::analyser::LoudnessAnalyser loudness(level);
    if(!audioFile.open_read(absoluteFilename.c_str()))
    {
        Loudness::io::AnalyseFile analyser(loudness, audioFile);
        analyser(checkProgress);
        audioFile.close();
    }
    checkDoubleValue(loudness.getIntegratedLoudness(), expectedIntegratedLoudnessValue);
    checkDoubleValue(loudness.getMaxShortTermLoudness(), expectedMaxShortTerm);
    checkDoubleValue(loudness.getMomentaryLoudness(), expectedMaxMomentary);
}

TEST(Case1kHzSine, Test20LUFS)
{
    checkEBUR128Analysis("1kHz Sine -20 LUFS-16bit.wav", -20, -20, -20);
}

TEST(Case1kHzSine, Test26LUFS)
{
    checkEBUR128Analysis("1kHz Sine -26 LUFS-16bit.wav", -26, -26, -26);
}

TEST(Case1kHzSine, Test40LUFS)
{
    checkEBUR128Analysis("1kHz Sine -40 LUFS-16bit.wav", -40, -40, -40);
}

int main(int argc, char** argv)
{
    // Initialize GTest system
    ::testing::InitGoogleTest(&argc, argv);

    // Get EBU_LOUDNESS_TEST_SET_PATH environment variable
    char* path = getenv("EBU_LOUDNESS_TEST_SET_PATH");
    if(path == NULL)
    {
        std::cerr << "Please set EBU_LOUDNESS_TEST_SET_PATH environment variable to launch unit tests." << std::endl;
        return EXIT_FAILURE;
    }
    ebuLoudnessTestSetPath = path;

    // Run GTests
    return RUN_ALL_TESTS();
}
