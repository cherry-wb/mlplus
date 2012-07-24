//
// Configuration class holds gdbt's configurations for training.
#ifndef MLPLUS_GDBT_CONFIGURATION_H
#define MLPLUS_GDBT_CONFIGURATION_H

#include <cstdio>
#include <string>

namespace mlplus
{
namespace gdbt
{
// Configuration
class Configuration
{
public:
    Configuration(const std::string &strConfigFile);
    ~Configuration();

    bool ParseConfiguration();

    int getTreeCount() const;
    int getMaxLeafCount() const;
    Loss::Type getType() const;
    std::string getTypeName() const;
    double getShrinkage() const;
    double getSampleRate() const;
    double getQuantileRate() const;
    bool IsFastTrain() const;
    Cut::DiscreteSeparatorType getDiscreteSeparatorType() const;
    double getMargin() const;
    double getVariableSampleRate() const;
    double getSplitBalance() const;
    int getMaxTreeDepth() const;
    int getMinLeafSampleCount() const;
    bool isSingleList() const;

private:
    // attributes.
    // General group
    int mnTreeCount;
    int mnMaxLeafCount;
    Loss::Type mLossType;	// Loss type
    double mfShrinkage;
    double mfSampleRate;
    bool mbFastTrain;	   // Define fast train level: [0-1]

    // Special group.
    double mfQuantileRate;		// Used for huber loss.

    // Configuration Parser.
    mllib::MlConfigParser *mpConfigParser;

    Cut::DiscreteSeparatorType msepType;
    double mfMargin;
    double mfVariableSampleRate;
    double mfSplitBalance;
    int mnMaxTreeDepth;
    int mnMinLeafSampleCount;
    bool mbSingleList;
}; // Configuration

} // gdbt
} // mlplus

#endif
