#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"
using namespace std;

// What this function does is that it takes in messageBits and a tree and follows the provided path in the message bits.
// Acknowledging that 0 = left and 1 = right, the function would dequeue the bit and process whether it should go left or right.
// If the function hits a leaf node, it would add the letter to the result and at the same time go back to the original root so that the function
// could continue decoding the tree.
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    string result = "";
    EncodingTreeNode* originalRoot = tree;

    while (!messageBits.isEmpty()){
        Bit currentBit = messageBits.dequeue();
        if (currentBit == 0){
            tree = tree->zero;
            if (tree->isLeaf() && tree->isLeaf()){ // checks to see if its a leaf node
                result += tree->ch;
                tree = originalRoot; // takes the function back to the original root of the leaf node
            }
        }
        else if (currentBit == 1 && tree->one){
            tree = tree->one;
        }
        else {
            break;
        }
        if (tree->zero == nullptr && tree->one == nullptr){ // checks to see if its a leaf node
            result += tree->ch;
            tree = originalRoot; // takes the function back to the original root of the leaf node
        }
    }

    return result;
}

// What this function does is unflatten the tree (aka creating the tree) by going through the queues of
// the treeShape input and treeLeaves input and iterating through to generate the tree.
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves){
    //check if its empty
    EncodingTreeNode* root = new EncodingTreeNode(nullptr, nullptr);

    Bit currentBit = treeShape.dequeue();

    if (treeShape.isEmpty() && treeLeaves.isEmpty()){ // first base case
        return root;
    }

    if (currentBit == 0){
        char currentChar = treeLeaves.dequeue();
        root->ch = currentChar;
    }

    else {
        root->zero = unflattenTree(treeShape, treeLeaves);
        root->one = unflattenTree(treeShape, treeLeaves);
    }

    return root;
}

// What this function does is decompress with the given data by going through each of the variables
// in the struct.
string decompress(EncodedData& data) {
    EncodingTreeNode* buildTree = unflattenTree(data.treeShape, data.treeLeaves);
    string message = decodeText(buildTree, data.messageBits);
    deallocateTree(buildTree); // clean up
    return message;
}

// this is a helper function for the main function buildHuffmanTree
// which creates a frequency map of the characters in the given text.
Map<char, int> freqMapHelper(string text){
    Map<char, int> result = {};
    for (char ch : text){
        result[ch] += 1;
    }
    return result;

// this is the important function in this program. What it does is build a huffman tree with the
// input of a text and initially sorts through a frequency map to see which characters have the most value
// then it goes through this priority queue with EncodingTreeNode data types and dequeueing the first two
// values which makes the node that'll be considered the main tree in the end.
EncodingTreeNode* buildHuffmanTree(string text) {
    if (text.size() <= 2){
        error("The input text must contain at least two distinct characters in order to build a Huffman encoding");
    }

    Map<char,int> frequencyMap = freqMapHelper(text);
    PriorityQueue<EncodingTreeNode*> result = {};

    for (char ch : frequencyMap){ // for each char in map
        int PQvalue = frequencyMap[ch];
        EncodingTreeNode* nodeChar = new EncodingTreeNode(ch);
        result.enqueue(nodeChar, PQvalue); // priority value
    }
    while (result.size() > 1){ // greater than one

        double firstPQNum = result.peekPriority();
        EncodingTreeNode* firstPQNode = result.dequeue();

        double secondPQNum = result.peekPriority();
        EncodingTreeNode* secondPQNode = result.dequeue();

        double totalPQNum = firstPQNum + secondPQNum;
        EncodingTreeNode* combinePQNode = new EncodingTreeNode(firstPQNode,secondPQNode);

        result.enqueue(combinePQNode, totalPQNum);
    }

    EncodingTreeNode* tree = result.dequeue();
    return tree;
}

// this is a helper function for encode text which intakes the tree, result, and bits
// in order to create the path for the characters as well as grabbing the letters in a map form.
void mapTree(EncodingTreeNode* tree, Map<char, string>& result, string bits){
    if (tree->isLeaf()){ // base case
        result[tree->ch] = bits;
        return;// basically add it to the map and get the char at child.
    }
    mapTree(tree->zero, result, bits + '0');
    mapTree(tree->one, result, bits + '1');
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a well-formed encoding tree and contains
 * an encoding for every character in the text.
 */

// this encodeText function goes takes in a tree and text and generates a queue of binary code that
// basically translates to the inputted text but in binary form. (could be considered cryptography.
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Map<char, string> result;
    Queue<Bit> resultQueue = {};
    mapTree(tree, result, "");
    for (char ch : text){
        string bits = result.get(ch);
        for (char num : bits){
            resultQueue.enqueue(num == '1');
        }
    }
    return resultQueue;
}

/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a well-formed encoding tree.
 */
// this function flattens the tree by intaking the tree, treeshape and tree leaves.
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (tree->isLeaf()){ // base case
        treeLeaves.enqueue(tree->ch);
        treeShape.enqueue(Bit(0));
        return;
    }
    treeShape.enqueue(Bit(1));
    flattenTree(tree->zero, treeShape, treeLeaves);
    flattenTree(tree->one, treeShape, treeLeaves);


}

/**
 * Compress the message text using Huffman coding, producing as output
 * an EncodedData containing the encoded message bits and flattened
 * encoding tree.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 */
// this function compresses the whole huffman tree
EncodedData compress(string messageText) {
    EncodedData data;

    EncodingTreeNode* tree = buildHuffmanTree(messageText);

    data.messageBits = encodeText(tree, messageText);

    flattenTree(tree, data.treeShape, data.treeLeaves);

    deallocateTree(tree);

    return data;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */

    EncodingTreeNode* letterE = new EncodingTreeNode('E');
    EncodingTreeNode* letterR = new EncodingTreeNode('R');
    EncodingTreeNode* letterS = new EncodingTreeNode('S');
    EncodingTreeNode* letterT = new EncodingTreeNode('T');
    EncodingTreeNode* root = new EncodingTreeNode(letterT, nullptr);
    root->one = new EncodingTreeNode(nullptr, letterE);
    root->one->zero = new EncodingTreeNode(letterR, letterS);

    return root;
}

// used post-order
void deallocateTree(EncodingTreeNode* t) {
    if (t){
        deallocateTree(t->zero);
        deallocateTree(t->one);
        delete t;
    }

}

void areEqualHelper(EncodingTreeNode* a, EncodingTreeNode* b, string& count){
    if (a->isLeaf() && b->isLeaf()){
        if ( a->ch == b->ch) {
            count += '1';
            return;
        }
        count += '0';
        return;
    }
    areEqualHelper(a->zero, b->zero, count);
    areEqualHelper(a->one, b->one, count);
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    string count = "";

    areEqualHelper(a, b, count);
    for (char ch : count){
        if (ch == '0'){
            return false;
        }
        return true;
    }
    return true;
}
