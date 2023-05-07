#ifndef random_forest_classifier_h
#define random_forest_classifier_h

#include "model_data.h" // Include data generated from MATLAB

int predictClass (float data[NUM_FEATURES]) {

    int treeVote[NUM_TREES] = {0};
    int predictedClass;
    int voteNumbers[NUM_CLASSES] = {0};    // Number of times each vote has been picked
    int pickedFlag;
    int currNode;   // MATLAB indexing at 1
    int prevNode;

    for (int i = 0; i < NUM_TREES; i++) {
        pickedFlag = 0; // Stating that the tree hasn't picked a class yet
        currNode = 0;   // Restart at first node
        while(!pickedFlag) {
            
            if (data[cutPredictor[i][currNode]] < cutPoints[i][currNode]) {    // CHANGE THESE INDICES
                currNode = children[i][currNode][0];
            }
            else {
                currNode = children[i][currNode][1];
            }

            if (children[i][currNode][1] == -1) {    // Children will say 0 when a class has been chosen
                pickedFlag = 1;
                predictedClass = nodeClass[i][currNode];
            }
        }

        treeVote[i] = predictedClass;     // Probably don't need this variable
        voteNumbers[predictedClass]++;    // Increment voting popularity

    }
    
    // Calculate most commonly voted class
    int popularClass = 0;
    for (int i = 1; i < NUM_CLASSES; i++) {
        if (voteNumbers[i] > voteNumbers[i-1]) {
            popularClass = i;
        }
    }
    return popularClass;

}

#endif