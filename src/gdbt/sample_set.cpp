#include <cstring>
#include <map>
#include "mlplus/interface/log.h"
#include "sample_set.h"
#include "sample_set_manager.h"
#include "mlplus/common/utility.h"
#include "sample_matrix.h"
#include "loss.h"
#include "cut.h"
#include "configuration.h"

namespace mlplus
{
namespace treelink
{
//ctor
SampleSet::SampleSet(SampleMatrix* pMatrix)
    : mpSampleMatrix(pMatrix), mfLoss(0.0), mnSampleCount(0), mpWeight(NULL),
      mnFullSampleCount(0), mpConf(NULL),
      mpManager(NULL), mbegin(this, 0), mend(this, pMatrix->GetSampleCount()),
      mbRoot(true), mbFastTrain(false)
{
    mnFullSampleCount = mpSampleMatrix->GetSampleCount();
    mnClassCount = mpSampleMatrix->GetClassCount();
    mpSelect = new bool[mnFullSampleCount];
    mpWeight = new float [mnFullSampleCount];
    mpEstimation = new float[mnFullSampleCount * mnClassCount];
    mppEstimation = new float * [mnFullSampleCount];
    mpGradient = new float[mnFullSampleCount * mnClassCount];
    mppGradient = new float * [mnFullSampleCount];

    // Reset buffers.
    float* pEst = mpEstimation;
    float* pGrad = mpGradient;
    for(int idxSample = 0; idxSample < mnFullSampleCount; ++idxSample)
    {
        mpSelect[idxSample] = false;
        mppEstimation[idxSample] = pEst; //mpEstimation + idxSample * mnClassCount;
        pEst += mnClassCount;
        mppGradient [idxSample] = pGrad; //mpGradient + idxSample * mnClassCount;
        pGrad += mnClassCount;
    }
    std::memset(mpEstimation, 0, sizeof(float) * mnClassCount * mnFullSampleCount);
    std::memset(mpGradient, 0, sizeof(float) * mnClassCount * mnFullSampleCount);

    // Initialize estimation, weight
    std::memcpy(mpEstimation, mpSampleMatrix->GetAllEstimation(), sizeof(float) * mnFullSampleCount * mnClassCount);
    if(mpSampleMatrix->GetWeighting())
        std::memcpy(mpWeight, mpSampleMatrix->GetAllWeight(), sizeof(float) * mnFullSampleCount);
    else
        for(int idxSample = 0; idxSample < mnFullSampleCount; ++idxSample)
            mpWeight[idxSample] = 1.0;
} // ctor

SampleSet::SampleSet(SampleSet& sampSet)
    : mpSampleMatrix(sampSet.mpSampleMatrix), mfLoss(sampSet.mfLoss), mnSampleCount(0), mpWeight(NULL),
      mnFullSampleCount(0), mpConf(sampSet.mpConf), mpManager(NULL), mbegin(this, 0), mend(this, sampSet.mpSampleMatrix->GetSampleCount()), mbRoot(false)
{
    mnFullSampleCount = mpSampleMatrix->GetSampleCount();
    mnClassCount = mpSampleMatrix->GetClassCount();
    mpSelect = new bool[mnFullSampleCount];
    mpWeight = sampSet.mpWeight;
    mpEstimation = sampSet.mpEstimation;
    mppEstimation = sampSet.mppEstimation;
    mpGradient = sampSet.mpGradient;
    mppGradient = sampSet.mppGradient;
}

SampleSetManager* SampleSet::GetManager()
{
    return mpManager;
}

void SampleSet::SetManager(SampleSetManager* pManager)
{
    mpManager = pManager;
}

bool SampleSet::CopyFrom(SampleSet& sampleSet)
{
    if(this != &sampleSet)
    {
        mfLoss = sampleSet.mfLoss;
        // std::memcpy(mpSelect, sampleSet.mpSelect, sizeof(bool) * mnFullSampleCount);
        mbFastTrain = sampleSet.mbFastTrain;
    }

    std::memset(mpSelect, 0, sizeof(bool) *mnFullSampleCount);
    mnSampleCount = 0;
    return true;
}

bool SampleSet::CopyFrom(SampleMatrix* pMatrix)
{
    mfLoss = 0.0;
    std::memcpy(mpEstimation, pMatrix->GetAllEstimation(),
                sizeof(float) * mnFullSampleCount * mnClassCount);
    if(pMatrix->GetWeighting())
    {
        std::memcpy(mpWeight, pMatrix->GetAllWeight(), sizeof(float) * mnFullSampleCount);
    }
    else
    {
        for(int idxSample = 0; idxSample < mnFullSampleCount; ++idxSample)
        {
            mpWeight[idxSample] = 1.0;
        }
    }
    std::memset(mpSelect, 0, sizeof(bool) *mnFullSampleCount);
    return true;
}

// dtor
SampleSet::~SampleSet()
{
    if(mpSelect)
        delete [] mpSelect;
    if(mbRoot)
    {
        if(mpEstimation)
            delete [] mpEstimation;
        if(mppEstimation)
            delete [] mppEstimation;
        if(mpGradient)
            delete [] mpGradient;
        if(mppGradient)
            delete [] mppGradient;
        if(mpWeight)
            delete [] mpWeight;
    }
} //dtor

float SampleSet::GetTarget(int nRowIndex) const
{
    return mpSampleMatrix->GetTarget(nRowIndex);
}

// Get the specific variable struct.
SampleSet::Variable* SampleSet::GetVariable(int nVarIndex)
{
    return new SampleSet::Variable(this, nVarIndex);
} // GetVariable()

// Get sample count in the sample set.
int SampleSet::GetSampleCount() const
{
    return mnSampleCount;
}

// Get variable type.
SampleMatrix::VariableType SampleSet::GetVariableType(int nVarIndex) const
{
    return mpSampleMatrix->GetVariableType(nVarIndex);
}

// Get class count.
int SampleSet::GetClassCount() const
{
    return mnClassCount;
}

// Get the begin-iterator
SampleSet::Iterator& SampleSet::Begin()
{
    return mbegin;
}

// Get the end-iterator.
SampleSet::Iterator& SampleSet::End()
{
    return mend;
}

// Get the full sample count (all samples in sample-matrix).
int SampleSet::GetFullSampleCount() const
{
    return mnFullSampleCount;
}

void SampleSet::SetSampleCount(int nSampleCount)
{
    mnSampleCount = nSampleCount;
}

int SampleSet::GetVariableCount() const
{
    return mpSampleMatrix->GetVariableCount();
}

// Calculate the best cut of the sample set.
Cut* SampleSet::CalculateBestCut(const Loss* pLoss)
{
    //UpdateGradient(pLoss);
    if(!mbFastTrain)
    {
        pLoss->CalculateGradient(this);
    }

    int nClassCount = GetClassCount();
    if(nClassCount <= 0)
    {
        return NULL;
    }
    Cut* pBestCut = new Cut(nClassCount);
    pBestCut->SetDistance(-1);
    Cut* pCut = new Cut(nClassCount);
    int nVarCnt = GetVariableCount();
    double* pGradSum = new double [nClassCount];
    int nTotalCount = 0;
    double fTotalWeight = 0;
    CalculateGradSum(pGradSum, nTotalCount, fTotalWeight);
    // variable stochastic
    for(int idxVar = 0; idxVar < nVarCnt; ++idxVar)
    {
        double fRand = static_cast<double>(std::rand()) / RAND_MAX;
        if(fRand >= mpConf->GetVariableSampleRate())
            continue;
        if(!CalculateVariableCut(idxVar, pGradSum, nTotalCount, fTotalWeight, pCut))
        {
            continue;
        }

        // The bigger distance is better!
        if(pBestCut->GetDistance() < pCut->GetDistance())
        {
            *pBestCut = *pCut;
        }
    }
    if(pBestCut->GetDistance() <= 0)
    {
        if(pBestCut)
            delete pBestCut;
        if(pCut)
            delete pCut;
        if(pGradSum)
            delete [] pGradSum;
        return NULL;
    }
    if(pCut)
        delete pCut;
    if(pGradSum)
        delete [] pGradSum;

    if(!mbFastTrain)
        CalculateCutGain(pLoss, pBestCut);
    else
        pBestCut->TakeDistanceAsGain();
    return pBestCut;
} // CalculateBestCut()

float SampleSet::GetVariableValue(int nSampleIndex, int nVarIndex) const
{
    return mpSampleMatrix->GetVariableValue(nSampleIndex, nVarIndex);
}

// Split the sample set using pCut.
bool SampleSet::Split(const Cut* pCut, SampleSet*& pLeftSampleSet, SampleSet*& pRightSampleSet)
{
    pLeftSampleSet = mpManager->TakeSampleSet(this);
    pRightSampleSet = mpManager->TakeSampleSet(this); // new SampleSet(*this);

    SampleMatrix::VariableType varType = pCut->GetVariableType();
    int nVarIndex = pCut->GetVariableIndex();
    const float* pLeftInc = NULL;
    const float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);
    int nClassCount = GetClassCount();
    for(int idxRow = 0; idxRow < mnFullSampleCount; ++idxRow)
    {
        if(mpSelect[idxRow])
        {
            bool bBranchLeft = false;
            if(varType == SampleMatrix::REAL)
            {
                if(pCut->BranchLeft(mpSampleMatrix->GetVariableValue(idxRow, nVarIndex)))
                {
                    bBranchLeft = true;
                }
            }
            else
            {
                int nVarValue = static_cast<int>(mpSampleMatrix->GetVariableValue(idxRow, nVarIndex));
                if(pCut->BranchLeft(nVarValue))
                {
                    // Nominal type.
                    bBranchLeft = true;
                }
            }

            if(bBranchLeft)
            {
                // Update left select-flags & estimations.
                pLeftSampleSet->mpSelect[idxRow] = true;
                if(!mbFastTrain)
                {
                    float* pEst = pLeftSampleSet->mppEstimation[idxRow];
                    float* pSrcEst = mpSampleMatrix->GetEstimation(idxRow);
                    CopyVector(pEst, pSrcEst, nClassCount);
                }
                ++pLeftSampleSet->mnSampleCount;
            }
            else
            {
                // Update left select-flags & estimations.
                pRightSampleSet->mpSelect[idxRow] = true;
                if(!mbFastTrain)
                {
                    float* pEst = pRightSampleSet->mppEstimation[idxRow];
                    float* pSrcEst = mpSampleMatrix->GetEstimation(idxRow);
                    CopyVector(pEst, pSrcEst, nClassCount);
                }
                ++pRightSampleSet->mnSampleCount;
            }
        } // if select
    } // for

    if(pLeftSampleSet->mnSampleCount == 0 || pRightSampleSet->mnSampleCount == 0)
    {
        pLeftSampleSet->GetManager()->PutSampleSet(pLeftSampleSet);
        pRightSampleSet->GetManager()->PutSampleSet(pRightSampleSet);
        TWARN("failed to split sample set, new sample sets have been put back.");
        return false;
    }
    return true;
}


void SampleSet::CalculateGradSum(double* pGradSum, int& nTotalCount, double& fTotalWeight)
{
    int nClassCount = GetClassCount();
    int nFullSampleCount = GetFullSampleCount();
    // Initialize sum & gradients.
    std::memset(pGradSum, 0, sizeof(double) * nClassCount);
    nTotalCount = 0;
    fTotalWeight = 0;
    // Add gradients completely to right-part.
    for(int idxRow = 0; idxRow < nFullSampleCount; ++idxRow)
    {
        if(mpSelect[idxRow])
        {
            // Selected in the sampleset.
            AddVector(pGradSum, mppGradient[idxRow], nClassCount);
            ++nTotalCount;
            fTotalWeight += mpWeight[idxRow];
        }
    }
}

bool SampleSet::CalculateVariableBest(int nVarIndex, double* pLeftGradSum,
                                      double* pRightGradSum, int nTotalCount, double fTotalWeight, float& fBestVar, float& fBestDist)
{
    int nFullSampleCount = GetFullSampleCount();
    int nClassCount = GetClassCount();
    int nLeftCount = 0;            // Left sample count
    double fLeftWeight = 0;
    float fLeftDist = 0.0;        // Distance of left gradient.
    int nRightCount = nTotalCount;
    double fRightWeight = fTotalWeight;
    int nLeftStart = static_cast<int>(nTotalCount * mpConf->GetSplitBalance());
    if(nLeftStart < mpConf->GetMinLeafSampleCount())
    {
        nLeftStart = mpConf->GetMinLeafSampleCount();
    }
    int nLeftEnd = nTotalCount - nLeftStart;
    if(nLeftEnd <= nLeftStart)
        return false;
    float fRightDist = DotVector(pRightGradSum, pRightGradSum, nClassCount);
    fRightDist    /= fRightWeight;
    fBestVar = 0.0;        // Best variable value.
    fBestDist = 0.0;        // Best current distance
    float fLastVar = 0.0;        // Last variable, keep unchanged if necessary.
    float fTotalDist = 0.0;
    float fSelfDist = fRightDist;

    const int* pVariableOrder = mpSampleMatrix->GetVariableOrder(nVarIndex);
    for(int idxVarRow = 0; idxVarRow < nFullSampleCount; ++idxVarRow)
    {
        int idxRow = pVariableOrder[idxVarRow];
        if(mpSelect[idxRow])
        {
            float fVar = mpSampleMatrix->GetVariableValue(idxRow, nVarIndex);
            if(nLeftCount >= nLeftStart && (fVar != fLastVar))
            {
                // Best distance should be updated.
                if(fLeftWeight <= 0)
                {
                    fLeftDist = 0.0;
                }
                else
                {
                    float fDotProd = DotVector(pLeftGradSum, pLeftGradSum, nClassCount);
                    fLeftDist = fDotProd / fLeftWeight;
                }

                if(fLeftWeight <= 0)
                {
                    fRightDist = 0.0;
                }
                else
                {
                    float fDotProd = DotVector(pRightGradSum, pRightGradSum, nClassCount);
                    fRightDist = fDotProd / fRightWeight;
                }
                fTotalDist = fLeftDist + fRightDist;

                if(fTotalDist > fBestDist)
                {
                    // Notice: larger distance(measure) is better, because it's a measure:)
                    fBestDist = fTotalDist;
                    fBestVar = (fLastVar + fVar) / 2.0;
                }
            }

            if(nLeftCount == nLeftEnd)
                break;

            // Update left & right sum (count)
            // Add current gradient to left part.
            AddVector(pLeftGradSum, mppGradient[idxRow], nClassCount);
            ++nLeftCount;
            fLeftWeight += mpWeight[idxRow];
            // Remove current gradient from right part.
            MinusVector(pRightGradSum, mppGradient[idxRow], nClassCount);
            --nRightCount;
            fRightWeight -= mpWeight[idxRow];

            // Update lastVar
            fLastVar = fVar;
        } // If selected.
    } // for varRow

    fBestDist -= fSelfDist;
    return fBestDist > 0;
}


bool SampleSet::CalculateVariableBest(int nVarIndex, double* pLeftGradSum,
                                      double* pRightGradSum, int nTotalCount, double fTotalWeight, int& nBestVar, float& fBestDist)
{
    int nFullSampleCount = GetFullSampleCount();
    int nClassCount = GetClassCount();
    int nLeftCount = 0;            // Left sample count
    double fLeftWeight = 0;
    float fLeftDist = 0.0;        // Distance of left gradient.
    int nRightCount = nTotalCount;
    double fRightWeight = fTotalWeight;
    float fRightDist = DotVector(pRightGradSum, pRightGradSum, nClassCount);
    fRightDist /= fRightWeight;
    nBestVar = 0;        // Best variable value.
    fBestDist = 0.0;        // Best current distance
    int nLastVar = 0;            // Integer format of last variable value.
    float fTotalDist = 0.0;
    float fSelfDist = fRightDist;

    const int* pVariableOrder = mpSampleMatrix->GetVariableOrder(nVarIndex);
    for(int idxVarRow = 0; idxVarRow < nFullSampleCount; ++idxVarRow)
    {
        int idxRow = pVariableOrder[idxVarRow];
        if(mpSelect[idxRow])
        {
            float fVar = mpSampleMatrix->GetVariableValue(idxRow, nVarIndex);
            int nVar = static_cast<int>(fVar);
            if(nLeftCount != 0 && (nVar != nLastVar))
            {
                // Best distance should be updated.
                if(fLeftWeight <= 0)
                {
                    fLeftDist = 0.0;
                }
                else
                {
                    float fDotProd = DotVector(pLeftGradSum, pLeftGradSum, nClassCount);
                    fLeftDist = fDotProd / fLeftWeight;
                }
                if(fRightWeight <= 0)
                {
                    fRightDist = 0.0;
                }
                else
                {
                    float fDotProd = DotVector(pRightGradSum, pRightGradSum, nClassCount);
                    fRightDist = fDotProd / fRightWeight;
                }

                fTotalDist = fLeftDist + fRightDist;

                if(fTotalDist > fBestDist)
                {
                    // Notice: larger distance(measure) is better, because it's a measure:)
                    fBestDist = fTotalDist;
                    nBestVar = nLastVar;
                }

                // left-part swap out to right-part.
                AddVector(pRightGradSum, pLeftGradSum, nClassCount);
                nRightCount += nLeftCount;
                fRightWeight += fLeftWeight;

                ResetVector(pLeftGradSum, nClassCount);    // Reset left sum vector
                nLeftCount = 0;
                fLeftWeight = 0;

            }
            // Update left & right sum (count)
            AddVector(pLeftGradSum, mppGradient[idxRow], nClassCount);
            ++nLeftCount;
            fLeftWeight += mpWeight[idxRow];
            MinusVector(pRightGradSum, mppGradient[idxRow], nClassCount);
            --nRightCount;
            fRightWeight -= mpWeight[idxRow];
            // update "last" variable
            nLastVar = nVar;
        } // If selected.
    } // for varRow

    fBestDist -= fSelfDist;
    return fBestDist > 0;
}


bool SampleSet::CalculateVariableBest(int nVarIndex, double* pLeftGradSum,
                                      double* pRightGradSum, int nTotalCount, double fTotalWeight, std::set<int>& setBestVar, float& fBestDist)
{
    int nFullSampleCount = GetFullSampleCount();
    int nClassCount = GetClassCount();
    int nLeftCount = 0;            // Left sample count
    double fLeftWeight = 0;
    float fLeftDist = 0.0;        // Distance of left gradient.
    int nRightCount = nTotalCount;
    double fRightWeight = 0;
    float fRightDist = DotVector(pRightGradSum, pRightGradSum, nClassCount);
    fRightDist /= fRightWeight;
    fBestDist = 0.0;        // Best current distance
    int nLastVar = 0;            // Integer format of last variable value.
    float fTotalDist = 0.0;
    float fSelfDist = fRightDist;

    const int* pVariableOrder = mpSampleMatrix->GetVariableOrder(nVarIndex);
    bool bFirstLoop = true;
    std::map<int, float*> mapValueGrad;
    std::map<int, float> mapValueWeight;
    int nMin(0);
    int nMax(0);
    for(int idxVarRow = 0; idxVarRow < nFullSampleCount; ++idxVarRow)
    {
        int idxRow = pVariableOrder[idxVarRow];
        if(mpSelect[idxRow])
        {
            float fVar = mpSampleMatrix->GetVariableValue(idxRow, nVarIndex);
            int nVar = static_cast<int>(fVar);
            // Record max and min of variable
            if(bFirstLoop)
            {
                bFirstLoop = false;
                nMin = nVar;
                nMax = nVar;
            }
            else
            {
                if(nVar < nMin)
                {
                    nMin = nVar;
                }
                else if(nVar > nMax)
                {
                    nMax = nVar;
                }
            }
            if(mapValueGrad.find(nVar) == mapValueGrad.end())
            {
                // Create element.
                float* pGrad = new float [nClassCount];
                ResetVector(pGrad, nClassCount);
                mapValueGrad[nVar] = pGrad;
                mapValueWeight[nVar] = 0.0;
            }
            AddVector(mapValueGrad[nVar], mppGradient[idxRow], nClassCount);
            mapValueWeight[nVar] += 1.0;
        } // end if select
    } // end for

    if(mapValueGrad.empty())
    {
        TERR("empty discrete value-map.");
        return false;
    }
    if(mapValueGrad.size() > 20)
    {
        TWARN("too many discrete value in a sigle variable.");
        return false;
    }


    // Search best
    // Use Gray code to simplify the computing.
    // Only one value change once.
    std::set<int> setLeft; // Left set.
    std::set<int> setRight; // Right set.
    std::vector<int> vecNominalIndex;
    for(std::map<int, float>::iterator iterValueWeight = mapValueWeight.begin();
            iterValueWeight != mapValueWeight.end(); ++iterValueWeight)
    {
        setRight.insert(iterValueWeight->first);
        vecNominalIndex.push_back(iterValueWeight->first);
    }
    setBestVar = setLeft;
    unsigned long nOriginalCode = 1;
    unsigned long nCurrGrayCode = common::utility::GrayCode(0);
    unsigned long nLastGrayCode = 0;
    int nSetSize = mapValueGrad.size();
    int nTotalPart = (1 << (nSetSize - 1)) - 1;
    const unsigned long nMask = static_cast<unsigned long>(-1) >> (sizeof(unsigned long) * 8 - nSetSize);

    // Search
    for(int idxPart = 0; idxPart < nTotalPart; ++idxPart)
    {
        nLastGrayCode = nCurrGrayCode;
        nCurrGrayCode = common::utility::GrayCode(nOriginalCode++) & nMask;
        // unsigned long nCurrCompCode = ~nCurrGrayCode;    //Complement Code
        // Calculate which value has a left-right transfer
        unsigned long nDiff = nCurrGrayCode ^ nLastGrayCode;
        unsigned long nDiffTemp = nDiff;
        //Calculate the position of "1"
        int nPosOne = -1;
        while(nDiffTemp)
        {
            nDiffTemp >>= 1;
            ++nPosOne;
        }
        // Find the index of the change.
        bool bLeftToRight = nDiff & nLastGrayCode;
        if(bLeftToRight)
        {
            // Move from left-set to right.
            setLeft.erase(vecNominalIndex[nPosOne]);
            MinusVector(pLeftGradSum, mapValueGrad[vecNominalIndex[nPosOne]], nClassCount);
            AddVector(pRightGradSum, mapValueGrad[vecNominalIndex[nPosOne]], nClassCount);
        }
        else
        {
            // Move from right-set to left-set
            setLeft.insert(vecNominalIndex[nPosOne]);
            AddVector(pLeftGradSum, mapValueGrad[vecNominalIndex[nPosOne]], nClassCount);
            MinusVector(pRightGradSum, mapValueGrad[vecNominalIndex[nPosOne]], nClassCount);
        }
        if(nLeftCount > 0)
        {
            fLeftDist = DotVector(pLeftGradSum, pLeftGradSum, nClassCount);
            fLeftDist /= fLeftWeight;
        }
        else
        {
            fLeftDist = 0.0;
        }
        if(nRightCount > 0)
        {
            fRightDist = DotVector(pRightGradSum, pRightGradSum, nClassCount);
            fRightDist /= fRightWeight;
        }
        else
        {
            fRightDist = 0.0;
        }
        fTotalDist = fLeftDist + fRightDist;
        // Update Best Cut
        if(fTotalDist > fBestDist)
        {
            // Notice: larger distance(measure) is better, because it's a measure:)
            setBestVar = setLeft;
            fBestDist = fTotalDist;
            nLastVar = -1;
        }
    } // End for (Gray search)
    // Delete all the grads in the map.
    for(std::map<int, float*>::iterator iter = mapValueGrad.begin(); iter != mapValueGrad.end(); ++iter)
    {
        if(iter->second)
            delete [] iter->second;
    }

    fBestDist -= fSelfDist;
    return !bFirstLoop;
}// End nominal:arbitrary

//
// Principle: maximize {sum{gradients of left region}^2/#{gradients of left region} +
// sum {gradients of right rigion}^2 / #{gradients of right redgion}
bool SampleSet::CalculateVariableCut(int nVarIndex, const double* pGradSum, int nTotalCount,
                                     double fTotalWeight, Cut* pCut)
{
    SampleMatrix::VariableType varType = GetVariableType(nVarIndex);
    if(varType == SampleMatrix::UNKNOWN)
    {
        TERR("unknown variable type.");
        return false;
    }

    int nClassCount = GetClassCount();
    double* pLeftGradSum = new double [nClassCount];
    ResetVector(pLeftGradSum, nClassCount);
    double* pRightGradSum = new double [nClassCount];
    ResetVector(pRightGradSum, nClassCount);
    CopyVector(pRightGradSum, pGradSum, nClassCount);

    // Save results to pCut
    pCut->SetVariableIndex(nVarIndex);
    pCut->SetVariableType(varType);
    float fBestDist = 0.0;
    if(varType == SampleMatrix::REAL)
    {
        float fBestVar = 0.0;
        if(!CalculateVariableBest(nVarIndex, pLeftGradSum, pRightGradSum, nTotalCount,
                                  fTotalWeight, fBestVar, fBestDist))
        {
            if(pLeftGradSum)
                delete [] pLeftGradSum;
            if(pRightGradSum)
                delete [] pRightGradSum;
            return false;
        }
        pCut->SetSeparator(fBestVar);
    }
    else
    {
        pCut->SetDiscreteSeparatorType(mpConf->GetDiscreteSeparatorType());
        if(mpConf->GetDiscreteSeparatorType() == Cut::LEAVE_ONE_OUT)
        {
            // Discrete separator type: leave-one-out
            int nBestVar = 0;
            if(!CalculateVariableBest(nVarIndex, pLeftGradSum, pRightGradSum, nTotalCount,
                                      fTotalWeight, nBestVar, fBestDist))
            {
                if(pLeftGradSum)
                    delete [] pLeftGradSum;
                if(pRightGradSum)
                    delete [] pRightGradSum;
                return false;
            }
            pCut->SetSeparator(nBestVar);
        }
        else
        {
            std::set<int> setBestVar;
            if(!CalculateVariableBest(nVarIndex, pLeftGradSum, pRightGradSum, nTotalCount,
                                      fTotalWeight, setBestVar, fBestDist))
            {
                TWARN("failed with ARBITRARY, try LEAVE_ONE_OUT.");
                int nBestVar = 0;
                if(!CalculateVariableBest(nVarIndex, pLeftGradSum, pRightGradSum,
                                          nTotalCount, fTotalWeight, nBestVar, fBestDist))
                {
                    if(pLeftGradSum)
                        delete [] pLeftGradSum;
                    if(pRightGradSum)
                        delete [] pRightGradSum;
                    TWARN("failed with LEAVE_ONE_OUT.");
                    return false;
                }
                else
                {
                    pCut->SetDiscreteSeparatorType(Cut::LEAVE_ONE_OUT);
                    pCut->SetSeparator(nBestVar);
                }
            }
            else
            {
                pCut->SetSeparator(setBestVar);
            }
        }
    }

    pCut->SetDistance(fBestDist);

    if(pLeftGradSum)
        delete [] pLeftGradSum;
    if(pRightGradSum)
        delete [] pRightGradSum;
    return true;
} //CalculateVariableCut()

// Calculate gain of the cut using pLoss
bool SampleSet::CalculateCutGain(const Loss* pLoss, Cut* pCut)
{
    SampleSet::Variable* pVariable = GetVariable(pCut->GetVariableIndex());
    bool bRet = pLoss->CalculateOptimal(pVariable, pCut);
    if(pVariable)
        delete pVariable;
    return bRet;
}

bool* SampleSet::GetSelect()
{
    return mpSelect;
}

float SampleSet::GetWeight(int nSampleIndex) const
{
    return mpSampleMatrix->GetWeight(nSampleIndex);
}

float SampleSet::GetOriginalWeight(int nSampleIndex) const
{
    return mpSampleMatrix->GetOriginalWeight(nSampleIndex);
}

void SampleSet::SetWeight(int nSampleIndex, float fWeight)
{
    mpSampleMatrix->SetWeight(nSampleIndex, fWeight);
}

SampleMatrix* SampleSet::GetSampleMatrix()
{
    return mpSampleMatrix;
}

float* SampleSet::GetEstimation(int nSampleIndex)
{
    return mppEstimation[nSampleIndex];
}

float* SampleSet::GetGradient(int nSampleIndex)
{
    return mppGradient[nSampleIndex];
}

// ctor of Sample::Iterator
SampleSet::Iterator::Iterator(SampleSet* pSampleSet, int nPos)
    : mnCurrentPos(nPos), mpSampleSet(pSampleSet)
{
}
// Copy ctor of Sample::Iterator
SampleSet::Iterator::Iterator(const Iterator& iter)
    : mnCurrentPos(iter.mnCurrentPos), mpSampleSet(iter.mpSampleSet)
{
}

// dtor of SampleSet::Iterator
SampleSet::Iterator::~Iterator()
{
}

// Copy assignment.
const SampleSet::Iterator& SampleSet::Iterator::operator = (const Iterator& iter)
{
    mpSampleSet = iter.mpSampleSet;
    mnCurrentPos = iter.mnCurrentPos;
    return *this;
}

// Iterator comparison
bool SampleSet::Iterator::operator == (const Iterator& iter) const
{
    return  mnCurrentPos == iter.mnCurrentPos && mpSampleSet == iter.mpSampleSet;
}

bool SampleSet::Iterator::operator != (const Iterator& iter) const
{
    return mnCurrentPos != iter.mnCurrentPos || mpSampleSet != iter.mpSampleSet;
}

// Prefix ++
const SampleSet::Iterator& SampleSet::Iterator::operator ++ ()
{
    do
    {
        ++mnCurrentPos;
    }
    while(mnCurrentPos < mpSampleSet->mnFullSampleCount  &&
            !mpSampleSet->mpSelect[mnCurrentPos]);
    return *this;
}

// Postfix++
const SampleSet::Iterator& SampleSet::Iterator::operator ++ (int)
{
    do
    {
        ++mnCurrentPos;
    }
    while(mnCurrentPos < mpSampleSet->mnFullSampleCount  &&
            !mpSampleSet->mpSelect[mnCurrentPos]);
    return *this;
}

// Get current target.
float SampleSet::Iterator::GetTarget() const
{
    return mpSampleSet->GetTarget(mnCurrentPos);
}

// Get current estimation.
float* SampleSet::Iterator::GetEstimation()
{
    return mpSampleSet->mppEstimation[mnCurrentPos];
}

// Get current gradient.
float* SampleSet::Iterator::GetGradient()
{
    return mpSampleSet->mppGradient[mnCurrentPos];
}

float SampleSet::Iterator::GetWeight() const
{
    return mpSampleSet->GetWeight(mnCurrentPos);
}

float SampleSet::Iterator::GetOriginalWeight() const
{
    return mpSampleSet->GetOriginalWeight(mnCurrentPos);
}

void SampleSet::Iterator::SetWeight(float fWeight)
{
    mpSampleSet->SetWeight(mnCurrentPos, fWeight);
}

// Variable::dtor()
SampleSet::Variable::Variable(SampleSet* pSampleSet, int nVarIndex)
    : mpSampleSet(pSampleSet), mnVariableIndex(nVarIndex), mbegin(this, 0), mend(this, pSampleSet->GetFullSampleCount())
{
    mpVariableOrder = mpSampleSet->mpSampleMatrix->GetVariableOrder(mnVariableIndex);
    mvariableType = mpSampleSet->mpSampleMatrix->GetVariableType(mnVariableIndex);
}

// Variable::dtor()
SampleSet::Variable::~Variable()
{
}

// Get the begin iterator
SampleSet::Variable::Iterator& SampleSet::Variable::Begin()
{
    return mbegin;
}

// Get the end iterator
SampleSet::Variable::Iterator& SampleSet::Variable::End()
{
    return mend;
}

// Get variable type.
SampleMatrix::VariableType SampleSet::Variable::GetVariableType() const
{
    return mvariableType;
}

// Get class count.
int SampleSet::Variable::GetClassCount() const
{
    return mpSampleSet->GetClassCount();
}

SampleSet* SampleSet::Variable::GetSampleSet() const
{
    return mpSampleSet;
}

bool SampleSet::Variable::GetWeighting() const
{
    return mpSampleSet->mpSampleMatrix->GetWeighting();
}

float SampleSet::Variable::GetWeight(int nPosition) const
{
    return mpSampleSet->GetWeight(mpVariableOrder[nPosition]);
}

void SampleSet::Variable::SetWeight(int nPosition, float fWeight)
{
    mpSampleSet->SetWeight(mpVariableOrder[nPosition], fWeight);
}

int SampleSet::Variable::GetSampleCount() const
{
    return mpSampleSet->GetSampleCount();
}

// Variable::Iterator
// Variable::Iterator::ctor()
SampleSet::Variable::Iterator::Iterator(Variable* pVariable, int nPos)
    : mpVariable(pVariable), mnCurrentPos(nPos)
{
}

// Copy ctor()
SampleSet::Variable::Iterator::Iterator(const Iterator& iter)
    : mpVariable(iter.mpVariable), mnCurrentPos(iter.mnCurrentPos)
{
}

// dtor()
SampleSet::Variable::Iterator::~Iterator()
{
}

const SampleSet::Variable::Iterator& SampleSet::Variable::Iterator::operator ++ ()
{
    do
    {
        ++mnCurrentPos;
    }
    while(mnCurrentPos < mpVariable->mpSampleSet->mnFullSampleCount  &&
            !mpVariable->mpSampleSet->mpSelect[mpVariable->mpVariableOrder[mnCurrentPos]]);
    return *this;
}

const SampleSet::Variable::Iterator& SampleSet::Variable::Iterator::operator ++ (int)
{
    do
    {
        ++mnCurrentPos;
    }
    while(mnCurrentPos < mpVariable->mpSampleSet->mnFullSampleCount  &&
            !mpVariable->mpSampleSet->mpSelect[mpVariable->mpVariableOrder[mnCurrentPos]]);
    return *this;
}

// Copy assignment.
const SampleSet::Variable::Iterator& SampleSet::Variable::Iterator::operator = (const Iterator& iter)
{
    mpVariable = iter.mpVariable;
    mnCurrentPos = iter.mnCurrentPos;
    return *this;
}

// Comparison operator ==.
bool SampleSet::Variable::Iterator::operator == (const Iterator& iter) const
{
    return mnCurrentPos == iter.mnCurrentPos && mpVariable == iter.mpVariable;
}

// Comparison operator !=
bool SampleSet::Variable::Iterator::operator != (const Iterator& iter) const
{
    return mnCurrentPos != iter.mnCurrentPos || mpVariable != iter.mpVariable;
}

// Get target value
float SampleSet::Variable::Iterator::GetTarget() const
{
    return mpVariable->mpSampleSet->GetTarget(mpVariable->mpVariableOrder[mnCurrentPos]);
}

// Get Estimation.
float* SampleSet::Variable::Iterator::GetEstimation()
{
    return mpVariable->mpSampleSet->GetEstimation(mpVariable->mpVariableOrder[mnCurrentPos]);
}

// Get Gradient.
float* SampleSet::Variable::Iterator::GetGradient()
{
    return mpVariable->mpSampleSet->GetGradient(mpVariable->mpVariableOrder[mnCurrentPos]);
}

// Get Variable as float format.
float SampleSet::Variable::Iterator::GetVariableDouble() const
{
    return mpVariable->mpSampleSet->GetVariableValue(mpVariable->mpVariableOrder[mnCurrentPos],
            mpVariable->mnVariableIndex);
}

// Get Variable as int format.
int SampleSet::Variable::Iterator::GetVariableInteger() const
{
    return static_cast<int>(mpVariable->mpSampleSet->GetVariableValue(
                                mpVariable->mpVariableOrder[mnCurrentPos],
                                mpVariable->mnVariableIndex
                            ));
}

float SampleSet::Variable::Iterator::GetWeight() const
{
    return mpVariable->GetWeight(mnCurrentPos);
}

void SampleSet::Variable::Iterator::SetWeight(float fWeight)
{
    mpVariable->SetWeight(mnCurrentPos, fWeight);
}

bool SampleSet::GetWeighting() const
{
    return mpSampleMatrix->GetWeighting();
}

void SampleSet::SetConfiguration(const Configuration* pConf)
{
    mpConf = pConf;
}

void SampleSet::SetFastTrain(bool bFastTrain)
{
    mbFastTrain = bFastTrain;
}

float* SampleSet::GetAllGradient() const
{
    return mpGradient;
}

const float* SampleSet::GetAllWeight() const
{
    return mpSampleMatrix->GetAllWeight();
}

} // treelink
} // mlplus

