#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <algorithm>
#define debug 0
static const int MAX_ITEMS = 100000;
static const float eps = 1.0e-10;
using namespace std;
/* 

   A typical ROC curve for reasonably accurate predictions looks 
   like this:

   |                                   *
   S   |                         *        
   E   |                 *
   N   |           *
   S   |               
   I   |       *  
   T   |       
   I   |    *
   V   |    
   I   |  *
   T   |  
   Y   |*
   - - - - - - - - - - - - - - - - - - - 
   1 - SPECIFICITY


   MODEL PREDICTION

   |       1       0       |
   - - - + - - - - - - - - - - - + - - - - -
   given         1   |       A       B       |    A+B
   OUTCOME           |                       |
   0   |       C       D       |    C+D
   - - - + - - - - - - - - - - - + - - - - -
   |      A+C     B+D      |  A+B+C+D


   1 = POSITIVE
   0 = NEGATIVE


   ACC = (A+D) /(A+B+C+D)
   PPV = A / (A+C)
   NPV = D / (B+D)
   SEN = A / (A+B)
   SPE = D / (C+D)


*/
struct Instance
{
    float pos;
    float neg;
    float pred;
    Instance(float a=0.0f, float b=0.0f, float c=0.0f):pos(a),neg(b), pred(c){}
    friend bool operator< (const Instance& a, const Instance& b)
    {   
         return a.pred < b.pred; 
    }
};
double accuracy(vector<Instance>& cases, double threshold)
{
    int no_item = cases.size();
    float a,b,c,d;
    int item;
    a = 0; b = 0; c = 0; d = 0;
    for (item=0; item<no_item; item++)
    {
            if (cases[item].pred >= threshold )
                a+=cases[item].pos;
            else
                b+=cases[item].pos;
            if (cases[item].pred>= threshold )
                c+=cases[item].neg;
            else
                d+=cases[item].neg;
    }
    return( ((double)(a+d)) / (((double)(a+b+c+d)) + eps) );
}

int main (int argc, char**argv)
{
    int plot = false;
    float no_item = 0;
    float mean_true = 0.0;
    float mean_pred = 0.0;
    float sse = 0.0;
    float pos = 0;
    float neg = 0;
    float p1 = 0;
    vector<Instance> cases;
    cases.reserve(10000);
    float pv, click;
    while (cin >> p1 >> pv >> click) 
    {
       /* Note that unlike the rest of the code, the RMSE calculation 
         assumes the class is 0 or 1 and that the probabilities 
         probably have been correctly normalized.
       */
        pos = click; //pv , click
        neg = pv - click;
        sse+= pos * (1-p1)*(1-p1) + neg * p1 * p1;
        mean_true+= pos + neg;
        mean_pred+= pos * p1 + neg * p1;
        no_item += pos + neg;
        cases.push_back(Instance(pos, neg, p1));
    }
    mean_true = mean_true / ((double) no_item);
    mean_pred = mean_pred / ((double) no_item);
    float rmse = sqrt (sse / ((double) no_item));
    if (debug)
    {
        printf("%d pats read. mean_true %6.4lf. mean_pred %6.4lf\n", cases.size(), mean_true, mean_pred);
        fflush(stdout);
    }

    float total_true_0 = 0;
    float total_true_1 = 0;
    for (int item=0; item<cases.size(); item++)
    {
        total_true_0+=cases[item].neg;
        total_true_1+=cases[item].pos;
    }
    /* sort data by predicted value */
    sort(cases.begin(), cases.end());
    /* now let's do the ROC cruve and area */

    float tp = 0; 
    float fn = total_true_1; 
    float fp = 0; 
    float tn = total_true_0;

    float tpr  = ((double) tp) / ((double) (tp+fn));
    float fpr = ((double) fp) / ((double) (fp+tn));
    if (plot)
        printf ("%6.4lf %6.4lf\n", fpr, tpr);
    float roc_area = 0.0;
    float tpr_prev = tpr;
    float fpr_prev = fpr;
    for (int item=cases.size()-1; item>-1; item--)
    {
        tp += cases[item].pos;  //true positive
        fn -= cases[item].pos;  //false negtive
        fp += cases[item].neg;  //false positive
        tn -= cases[item].neg;  
        tpr = ((double) tp) / ((double) (tp+fn));
        fpr  = ((double) fp) / ((double) (tn+fp));
        if ( item > 0 )
            if (cases[item].pred!= cases[item-1].pred)
            {
                if ( plot )
                    printf ("%6.4lf %6.4lf\n", fpr, tpr);
                roc_area+= 0.5*(tpr+tpr_prev)*(fpr-fpr_prev);
                tpr_prev = tpr;
                fpr_prev = fpr;
            }
        if ( item == 0 )
        {
            if ( plot )
                printf ("%6.4lf %6.4lf\n", fpr, tpr);
            roc_area+= 0.5*(tpr+tpr_prev)*(fpr-fpr_prev);
        }
    }
    printf ("ACC %8.5lf\n", accuracy(cases, 0.5));
    printf ("RMS %8.5lf\n", rmse);
    printf ("ROC %8.5lf\n", roc_area);
}
