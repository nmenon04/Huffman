/*Nora Menon
* 106B
* This code builds up towards compressing and decompressing files based on Huffman trees.
* Through this code, files, images, and text documents can be compressed to save space and then the process can be reversed
* to bring it back to regular size.
*/

#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"  // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;

/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 *
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    string phrase = "";
    EncodingTreeNode* current = tree;
    while(messageBits.size() > 0) {
        while(current->zero != nullptr && current->one != nullptr) {
            Bit first = messageBits.dequeue();
            if(first == 0) {
                current = current->zero;
            }
            else {
                current = current->one;
            }
        }
        phrase += current->ch;
        current = tree;
    }
    return phrase;
}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 *
 *
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if(treeShape.size() == 0 || treeLeaves.size() == 0) {
        return nullptr;
    }

    Bit node = treeShape.dequeue();
    if(node == 0) {
        char value = treeLeaves.dequeue();
        return new EncodingTreeNode(value);
    }

    EncodingTreeNode* leftChild = unflattenTree(treeShape, treeLeaves);
    EncodingTreeNode* rightChild = unflattenTree(treeShape, treeLeaves);

    EncodingTreeNode* root = new EncodingTreeNode(leftChild, rightChild);
    return root;
}

/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 *
 * Combines previous helper functions
 */
string decompress(EncodedData& data) {
    EncodingTreeNode* unflattened = unflattenTree(data.treeShape, data.treeLeaves);
    string uncoded = decodeText(unflattened, data.messageBits);
    deallocateTree(unflattened);
    return uncoded;
}

/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * Utilizes prioritt queues
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    if(text.size() < 2) {
        error("There is less that two characters in this text!");
    }

    //makes sure there are at least 2 distinct characters
    bool moreThanTwo = false;
    int distincts = 0;
    for(char c: text) {
        moreThanTwo = (text[0] != c);
        if(moreThanTwo) {
            distincts++;
        }
    }
    if(distincts == 0) {
        error("There is less that two distinct characters in this text!");
    }


    Map<char, int> map;
    for(char c: text) {
        if(!(map.containsKey(c))) {
            map.put(c, 1);
        }
        else {
            map[c]++;
        }
    }

    PriorityQueue<EncodingTreeNode*> trees;
    for(char c: map) {
        trees.enqueue(new EncodingTreeNode(c), map[c]);
    }

    while(trees.size() > 1) {
        int frequency0 = trees.peekPriority();
        EncodingTreeNode* left = trees.dequeue();
        int frequency1 = trees.peekPriority();
        EncodingTreeNode* right = trees.dequeue();

        EncodingTreeNode* parent = new EncodingTreeNode(left, right);

        //parent will be automatically sorted into the right spot because of the priority queue
        trees.enqueue(parent, frequency0 + frequency1);

    }

    return trees.peek();
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 *
 *
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Queue<Bit> output;
    Vector<Bit> sequence;
    Map<char, Vector<Bit>> map;
    encodeHelper(tree, sequence, map);

    for(char c: text) {
        Vector<Bit> bits = map[c];
        while (bits.size() != 0) {
            output.enqueue(bits.get(0));
            bits.remove(0);
        }
    }
    return output;
}

//helper function that traverses through a tree and builds a map of the characters and their associated bit sequence
void encodeHelper(EncodingTreeNode* tree, Vector<Bit> sequence, Map<char, Vector<Bit>>& map) {
    if(tree == nullptr) {
        return;
    }
    if(tree->isLeaf()) {
        map.put(tree->getChar(), sequence);
        return;
    }
    sequence.add(0);
    encodeHelper(tree->zero, sequence, map);
    sequence.remove(sequence.size()-1);
    sequence.add(1);
    encodeHelper(tree->one, sequence, map);
}

/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 *
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if(tree == nullptr) return;
    if(tree->isLeaf()) {
        treeShape.enqueue(0);
        treeLeaves.enqueue(tree->ch);
        return;
    }
    treeShape.enqueue(1);
    flattenTree(tree->zero, treeShape, treeLeaves);
    flattenTree(tree->one, treeShape, treeLeaves);
}

/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 *
 */
EncodedData compress(string messageText) {
    EncodedData compressed;
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    Queue<Bit> messageBits;

    EncodingTreeNode* tree = buildHuffmanTree(messageText);
    flattenTree(tree, treeShape, treeLeaves);
    messageBits = encodeText(tree, messageText);

    //assigned the queues to the EncodedData
    compressed.treeLeaves = treeLeaves;
    compressed.treeShape = treeShape;
    compressed.messageBits = messageBits;

    deallocateTree(tree);
    return compressed;

}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

//creates a specific tree that has TRSE as the leaves
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
    Vector<EncodingTreeNode*> leaves;
    EncodingTreeNode* T = new EncodingTreeNode('T');
    EncodingTreeNode* R = new EncodingTreeNode('R');
    EncodingTreeNode* S = new EncodingTreeNode('S');
    EncodingTreeNode* E = new EncodingTreeNode('E');

    EncodingTreeNode* inter1 = new EncodingTreeNode(R, S);
    EncodingTreeNode* inter2 = new EncodingTreeNode(inter1, E);
    EncodingTreeNode* root = new EncodingTreeNode(T, inter2);
    return root;
}

//deletes all the nodes in a tree
void deallocateTree(EncodingTreeNode* t) {
    if(t==nullptr) {
        return;
    }
    deallocateTree(t->zero);
    deallocateTree(t->one);
    delete t;
}

//tests if two trees are identical
bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    if(a==nullptr && b == nullptr) {
        return true;
    }
    if(a==nullptr || b == nullptr) {
        return false;
    }

    if(a->ch != b->ch) {
        return false;
    }

    bool equalLeft = areEqual(a->zero, b->zero);
    bool equalRight = areEqual(a->one, b->one);

    return equalLeft && equalRight;
}

/* * * * * * Test Cases Below This Point * * * * * */


STUDENT_TEST("proves that createExampleTree and deallocateTree work") {
    EncodingTreeNode* root = createExampleTree();
    deallocateTree(root);
}

STUDENT_TEST("Compare two empty trees, then comparing an empty tree to a tree with children, then proves the parent replica with the children switched is not equal") {
    EncodingTreeNode* root1 = nullptr;
    EncodingTreeNode* root2 = nullptr;
    EXPECT(areEqual(root1, root2));

}
STUDENT_TEST("empty to not empty & shows that the not empty is equal to a replica of itself") {
    EncodingTreeNode* empty = nullptr;

    EncodingTreeNode* child1 = new EncodingTreeNode('Z');
    EncodingTreeNode* child2 = new EncodingTreeNode('O');
    EncodingTreeNode* parent = new EncodingTreeNode(child1, child2);

    EXPECT_EQUAL(areEqual(empty, parent), false);

    EncodingTreeNode* childReplica1 = new EncodingTreeNode('Z');
    childReplica1 = child1;
    EncodingTreeNode* childReplica2 = new EncodingTreeNode('Z');
    childReplica2 = child2;
    EncodingTreeNode* parentReplica = parent;
    EncodingTreeNode* parentFlipped = new EncodingTreeNode(childReplica2, childReplica1);
    EXPECT_EQUAL(areEqual(parentReplica, parent), true);
    EXPECT_EQUAL(areEqual(parentFlipped, parent), false);



    deallocateTree(empty);
    deallocateTree(parent);
    deallocateTree(parentFlipped);

}

STUDENT_TEST("SImple Tree and createExampleTree trials") {
    EncodingTreeNode* child1 = new EncodingTreeNode('Z');
    EncodingTreeNode* child2 = new EncodingTreeNode('O');
    EncodingTreeNode* parent = new EncodingTreeNode(child1, child2);

    EncodingTreeNode* trse = createExampleTree();
    EXPECT_EQUAL(areEqual(trse, parent), false);

    //another trse tree is created
    EncodingTreeNode* trse2 = createExampleTree();
    EXPECT_EQUAL(areEqual(trse, trse2), true);

    EXPECT_EQUAL(areEqual(child1, parent), false);

    deallocateTree(parent);
    deallocateTree(trse);
    deallocateTree(trse2);

}

STUDENT_TEST("decodeText, small example encoding tree") {
    Vector<EncodingTreeNode*> leaves;
    EncodingTreeNode* S = new EncodingTreeNode('S');
    EncodingTreeNode* T = new EncodingTreeNode('T');
    EncodingTreeNode* R = new EncodingTreeNode('R');
    EncodingTreeNode* E = new EncodingTreeNode('E');

    EncodingTreeNode* inter1 = new EncodingTreeNode(T, R);
    EncodingTreeNode* inter2 = new EncodingTreeNode(inter1, E);
    EncodingTreeNode* root = new EncodingTreeNode(S, inter2);

    EXPECT(root != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(root, messageBits), "E");

    messageBits = { 0, 1, 1, 1, 0, 0 }; // SET
    EXPECT_EQUAL(decodeText(root, messageBits), "SET");

    messageBits = { 1, 0, 0, 1, 0, 1, 1, 1, 0, 0}; // TRESS
    EXPECT_EQUAL(decodeText(root, messageBits), "TRESS");

    Map<char, Vector<Bit>> map;
    Vector<Bit> seq;
    encodeHelper(root, seq, map);

    deallocateTree(root);
}

STUDENT_TEST("Decrompressing a tree into a string") {
    EncodedData data = {
        { 1, 1, 0, 1, 0, 0, 1, 0, 0 }, // treeShape
        { 'F', 'L', 'E', 'R', 'A' },  // treeLeaves
        { 1, 0, 0, 1, 1, 1, 1, 0, 1, 0 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "REAL");
}

STUDENT_TEST("Flatten then unflatten") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* copy = reference;
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);
    EncodingTreeNode* back = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(copy, back));

    deallocateTree(copy);
    deallocateTree(back);
}

STUDENT_TEST("buildHuffmanTree, example encoding tree") {
    EncodingTreeNode* reference = createExampleTree();
    EncodingTreeNode* tree = buildHuffmanTree("SSTTTTREEE");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

STUDENT_TEST("Encode then Decode") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    Queue<Bit> encoded = encodeText(reference, "E");
    string decoded = decodeText(reference, encoded);

    EXPECT_EQUAL(decoded, "E");

    deallocateTree(reference);
}

STUDENT_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "13HFSEHFD83NKQRFJS",
        "hI HI Hi",
        "I love pasta.",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman",
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
