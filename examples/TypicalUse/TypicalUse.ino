/*
 * Example to demonstrate RepetitiveRamReviewer
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!! Warning: This program is supposed to crash after a while !!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * RepetitiveRamReviewer will show you the free memory.
 * If it runs out, the program can behave unpredictable.
 *
 * This is critical log message from RepetitiveRamReviewer:
 *
 *   Here 0 bytes RAM left _________
 *                                  \/
 *     |[01dc]==( 882)=>|[054e]=>(   0)<=|[054e]<=( 945)==[08ff]|
 *                /\                                /\
 *                |                                 |
 *   Heap uses 882 bytes                            |
 *   Stack uses at least 945 bytes -----------------+
 *
 * by Frank Müller
 *
 * MIT License.
 */

#include <RepetitiveRamReviewer.h>

RepetitiveRamReviewer rrr;

void setup() {
  rrr.begin();

  // Don't forget to set the same baud rate in serial monitor
  Serial.begin(115200);
  Serial.println(F("(Re)start"));

  // Do your initializing here
  pinMode(LED_BUILTIN, OUTPUT);
  randomSeed(analogRead(0));
}

void loop() {
  // Do your stuff here
  stuff();

  // Meassue and dump every 2s
  rrr.loopAndDump(2000);
  // Or meassue every 500ms but dump only on change
  //rrr.loopAndDumpChanges(500);
}

// The code below is just one way to crash the program

struct Node {
  Node *parent;
  int nr;
  int value;
  int sum;
  Node *left;
  Node *right;
};

enum State {
  Start,
  CreateTree,
  CalcSum,
  KillMeSlowly
};

State state = Start;
Node *root = NULL;
Node *curNode = NULL;
int curLevel = 0;
const int MAX_LEVEL = 5;
int maxDeep = 10;


Node *createNode(Node *parent);
int calcSum(Node *node);
void printTree(Node *node);
int getMaxLevel(Node *node, int curLevel = 1);
void printTreeLevel(Node *node, int level);

Node *createNode(Node *parent) {
  static int count = 0;
  Node *newNode = new Node();

  if (newNode) {
    newNode->parent = parent;
    newNode->nr = count;
    ++count;
    newNode->value = random(10);
    newNode->sum = 0;
    newNode->left = newNode->right = NULL;

    if (parent) {
      if (parent->left == NULL) {
        parent->left = newNode;
      } else {
        parent->right = newNode;
      }
    }
  } else {
    Serial.println(F("Not enough memory, program may crash here"));
  }

  return newNode;
}


void stuff() {
  if (random(500)) return;

  switch (state) {
    case Start:
      Serial.println(F("Create a tree on heap"));
      state = CreateTree;
      break;
    case CreateTree:
      if (root == NULL) {
        root = createNode(NULL);
        curNode = root;
      } else if (curNode != NULL) {
        if ((curLevel < MAX_LEVEL) && ((curNode->left == NULL) || (curNode->right == NULL))) {
          curNode = createNode(curNode);
          ++curLevel;
        } else {
          curNode = curNode->parent;
          --curLevel;
        }
      } else {
        Serial.println(F("A tree is grown"));
        state = CalcSum;
      }
      break;
    case CalcSum:
      {
        int sum = calcSum(root);
        Serial.print(F("Weight of the tree: "));
        Serial.println(sum);
        printTree(root);
        state = KillMeSlowly;
      }
      break;
    case KillMeSlowly:
      if (random(50)) return;
      if (random(2)) {
        digitalWrite(LED_BUILTIN, HIGH);
        crashTheStack(random(maxDeep/10, maxDeep));
        Serial.println('!');
        maxDeep += 10;
      } else {
        digitalWrite(LED_BUILTIN, LOW);
        int sum = calcSum(root);
        Serial.print(F("Weight of the tree: "));
        Serial.println(sum);
        printTree(root);
      }
      break;
  }
}

int calcSum(Node *node) {
  if (node) {
    return node->sum = node->value + calcSum(node->left) + calcSum(node->right);
  } else {
    return 0;
  }
}

void printTree(Node *node) {
  if (node) {
    int max = getMaxLevel(node);
    for (int level = 1; level <= max; ++level) {
      int indent = (1 << (max-level+1)) - 2;
      for (int i = 0; i < indent; ++i) {
        Serial.print(' ');
      }
      printTreeLevel(node, level);
      Serial.println();
    }
  }
}

int getMaxLevel(Node *node, int curLevel = 1) {
  if (node) {
    int level = curLevel;
    int tmp = getMaxLevel(node->left, curLevel+1);
    if (tmp > level) level = tmp;
    tmp = getMaxLevel(node->right, curLevel+1);
    if (tmp > level) level = tmp;
    return level;
  } else {
    return 0;
  }
}

void printTreeLevel(Node *node, int level) {
  if (node) {
    if (level == 1) {
      if (node->nr < 10) Serial.print(' ');
      Serial.print(node->nr);
      int max = getMaxLevel(node);
      max = (1 << (max+1)) - 2;
      for (int i = 0; i < max; ++i) {
        Serial.print(' ');
      }
    } else {
      printTreeLevel(node->left, level-1);
      printTreeLevel(node->right, level-1);
    }
  }
}

void crashTheStack(int deep) {
  if (deep > 0) {
    crashTheStack(deep - 1);
  }
  Serial.print(deep);
  Serial.print('.');
}
