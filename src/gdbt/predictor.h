//
// Predictor class.
//
#ifndef MLLIB_TREELINK_PREDICTOR_H_
#define MLLIB_TREELINK_PREDICTOR_H_

#include <string>
#include <iostream>


#include "model_manager.h"

namespace mlplus
{
namespace treelink
{

class TreeBoost;

/**Predictor class. It encapsulate the predict process.
 */
class Predictor
{
public:
    // Trival ctor and dtor.
    Predictor();

    /**Predictor constructor.
     * @param pTreeBoost A pointer to TreeBoost object, which is the core of the predictor
     * If the object is built, then you can use it as the core directly.
     */
    Predictor(TreeBoost* pTreeBoost);

    ~Predictor();

    /**Predict function.
     * @param x A reference to feature vector, it is dense.
     * @param y The predicted target. If the predictor is a classifier, the return value is
     * the ID of the category; if the predictor is a regressor, the return value is the regress
     * value. Notice that if the regressor (classifier) is AdaBoost, then the return value is
     * a real number between -1 and 1.
     * @return Whether procedure succeeds.
     */
    bool Predict(const std::vector<float>& x, float& y) const;

    /**Predict function.
     * @param x A reference to feature vector, it is dense.
     * @param y A writable reference to distribution vector. If the predictor is a classifier,
     * y contains the probability distribution, if the predictor is a regressor, y contains
     * the regress value.
     * @return Whether the predicting process succeeds.
     */
    bool Predict(const std::vector<float>& x, std::vector<float>& y) const;

    /**Do variable importance analysis.
     * @param vecVarImp A writable reference to a vector object, which contains the importance
     * of the variable accordingly.
     * @return Whehter the process succeeds.
     */
    bool AnalyzeVariableImportance(std::vector<float>& vecVarImp);

    double CalculateLabelValue(const std::vector<float>& x) const;

    /**Load a model.
     * @param is A reference to input stream.
     * @return Whether the procedure succeeds.
     */
    bool Load(std::istream& is);

    /**Save a model.
     * @param os A reference to output stream.
     * @return Whether the procedure succeeds.
     */
    bool Store(std::ostream& os);

    /**Clear the model.
     */
    bool Clear();

    bool SetUsedTreeCount(size_t nUsedTreeCount);

protected:
    TreeBoost* GetTreeBoost();

private:
    TreeBoost* mpTreeBoost;
    friend class ModelManager;
    friend class CFileModelManager;
    friend class ParameterModelManager;
    friend class mlplus::TreeLink;
};// Predictor

} // treelink
} // mlplus

#endif // MLLIB_TREELINK_PREDICTOR_H_

