
#include <vector>
#include <algorithm>

#include <iostream>

#ifndef byte
#define byte unsigned char
#endif

template<typename T, class N = double>
class RectTree {
public:
  enum NodePlace {
    TL = 0, TR = 1, BL = 2, BR = 3, INVALID = 4
  };
  class Rect;
  class RectResult;
  class Node;

public:
  class Rect {
    public:
      N x, y, w, h;
      T value;
      inline Rect(N _x, N _y, N _w, N _h, T _value)
        : x(_x), y(_y), w(_w), h(_h), value(_value)
      {
        // 
      }
  };

public:
  class Node {
    public:
      inline Node(
        const RectTree &_tree,
        const Node &_parent,
        int _depth, int _maxDepth, byte _place
      )
        : rectList()
        , tree(_tree)
        , parent(_parent)
        , depth(_depth)
        , maxDepth(_maxDepth)
        , place(_place)
        // , x(0), y(0), w(0), h(0)
      {

        x = parent.x+((place == TR || place == BR) ? parent.w/2 : 0.0);
        y = parent.y+((place == BL || place == BR) ? parent.h/2 : 0.0);
        w = parent.w/2;
        h = parent.h/2;

        for(int i = 0; i < depth; i++) {
          std::cout << "-";
        }
        std::cout << " inst: " << (int)place << "; " << x << ", " << y << ", " << w << ", " << h << std::endl;

        if(depth != _maxDepth) {
          children.push_back(Node(tree, *this, depth+1, _maxDepth, TL));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, TR));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, BL));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, BR));
        }
      }

      inline Node( // root node constructor
        const RectTree &_tree,
        int _maxDepth,
        N _x, N _y, N _w, N _h
      ) : tree(_tree), parent(*((Node*)NULL))
        , place(INVALID)
        , x(_x), y(_y), w(_w), h(_h)
        , depth(0), maxDepth(_maxDepth)
      {
        std::cout << "inst: " << (int)place << "; " << x << ", " << y << ", " << w << ", " << h << std::endl;
        if(depth != _maxDepth) {
          children.push_back(Node(tree, *this, depth+1, _maxDepth, TL));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, TR));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, BL));
          children.push_back(Node(tree, *this, depth+1, _maxDepth, BR));
        }
      }

    public:
      std::vector<Rect> rectList;

      inline bool isLeaf() const
      {
        return (depth == maxDepth);
      }

      inline bool contains(const Rect &rect) const
      {
        return (
              rect.x >= x && rect.x+rect.w <= x+w
          &&  rect.y >= y && rect.y+rect.h <= y+h
        );
      }
      inline bool containsPoint(N px, N py) const
      {
        return (
              px >= x && px <= x+w
          &&  py >= y && py <= y+h
        );
      }

      inline bool add(Rect &rect)
      {
        if(contains(rect)) {
          //std::cout << (int)place << ".";
          if(
                !isLeaf()
            &&  (
                      children[TL].add(rect)
                  ||  children[TR].add(rect)
                  ||  children[BL].add(rect)
                  ||  children[BR].add(rect)
                )
          ) {
            return true;
          }
          rectList.push_back(rect);
          return true;
        }
        return false;
      }

      inline void retrieveAllRects(std::vector<RectResult> &resList)
      {
        typename std::vector<Rect>::iterator iter = rectList.begin();
        for(; iter != rectList.end(); iter++) {
          Rect &rect = *iter;
          resList.push_back(RectResult(rect, *this, iter-rectList.begin()));
        }
        if(!isLeaf()) {
          children[TL].retrieveAllRects(resList);
          children[TR].retrieveAllRects(resList);
          children[BL].retrieveAllRects(resList);
          children[BR].retrieveAllRects(resList);
        }
      }

      inline void findRectsAtPoint(N px, N py, std::vector<RectResult> &resList)
      {
        // std::cout << "push: " << (int*)this << " -- ";
        typename std::vector<Rect>::iterator iter = rectList.begin();
        for(; iter != rectList.end(); iter++) {
          Rect &rect = *iter;
          if(
                px >= rect.x && px <= rect.x+rect.w
            &&  py >= rect.y && py <= rect.y+rect.h
          ) {
            resList.push_back(RectResult(rect, *this, iter-rectList.begin()));
            // std::cout << iter-rectList.begin() << "; ";
          }
        }
        // std::cout << std::endl;
        if(!isLeaf()) {
          if(children[TL].containsPoint(px, py)) {
            children[TL].findRectsAtPoint(px, py, resList);
          } else if(children[TR].containsPoint(px, py)) {
            children[TR].findRectsAtPoint(px, py, resList);
          } else if(children[BL].containsPoint(px, py)) {
            children[BL].findRectsAtPoint(px, py, resList);
          } else if(children[BR].containsPoint(px, py)) {
            children[BR].findRectsAtPoint(px, py, resList);
          }
        }
      }

    public:
      const RectTree &tree;
      const Node &parent;

      const int depth, maxDepth;
      const byte place;

      /*const*/ N x, y, w, h;

      std::vector<Node> children;
  };

public:
  class RectResult {
    public:
      Rect *rect;
      Node *node;
      int numInNode;
      inline RectResult(Rect &_rect, Node &_node, int _num)
        : rect(&_rect), node(&_node), numInNode(_num)
      {
        //
      }
      inline bool operator==(const RectResult &other) const
      {
        return (node == other.node && numInNode == other.numInNode);
      }
      inline bool operator<(const RectResult &other) const
      {
        return !(numInNode < other.numInNode);
      }
  };

public:
  inline RectTree(N x, N y, N w, N h, int maxDepth)
  {
    rootNodePtr = new Node(*this, maxDepth, x, y, w, h);
  }

  inline void reset(N x, N y, N w, N h, int maxDepth)
  {
    delete rootNodePtr;
    rootNodePtr = new Node(*this, maxDepth, x, y, w, h);
  }

  inline bool add(Rect &rect) {
    //std::cout << "add: ";
    bool res = rootNodePtr->add(rect);
    //std::cout << std::endl;
    return res;
  }
  inline void retrieveAllRects(std::vector<RectResult> &resList) {
    rootNodePtr->retrieveAllRects(resList);
  }
  inline void findRectsAtPoint(N px, N py, std::vector<RectResult> &resList)
  {
    rootNodePtr->findRectsAtPoint(px, py, resList);
  }
  inline Node &getNodeAtPoint(N px, N py)
  {
    Node *currNode = rootNodePtr;
    int depth = 0;
    while(!currNode->isLeaf() && depth < 50) {
      currNode = (
          currNode->children[TL].containsPoint(px, py) ? &currNode->children[TL]
        : currNode->children[TR].containsPoint(px, py) ? &currNode->children[TR]
        : currNode->children[BL].containsPoint(px, py) ? &currNode->children[BL]
        : currNode->children[BR].containsPoint(px, py) ? &currNode->children[BR]
        : currNode // ?!?
      );
      depth++;
    }
    return *currNode;
  }
  inline void removeResults(std::vector<RectResult> &resList)
  {
    resList.erase(std::unique(resList.begin(), resList.end()), resList.end());
    std::sort(resList.begin(), resList.end());
    // must be in descending order for indices to remain valid during erasure
    typename std::vector<RectResult>::iterator iter = resList.begin();
    for(; iter != resList.end(); iter++) {
      RectResult &res = *iter;
      //std::cout << "erase: " << res.numInNode << "/" << res.node->rectList.size() << std::endl;
      res.node->rectList.erase(
        res.node->rectList.begin()+res.numInNode
      );
    }
    resList.clear();
  }

private:
  Node *rootNodePtr;
  //std::vector<Node> nodeList;
};

