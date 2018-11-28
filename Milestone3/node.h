#ifndef __NODE_H_
#define __NODE_H_

class Node
{
  public:
    uint8_t x;
    uint8_t y;
    int cost;
    Node *parent;
    Node *next;
    Node *prev;
    Node(uint8_t xCoor, uint8_t yCoor, int c, Node *node1, Node *node2, Node *node3)
    {
      x = xCoor;
      y = yCoor;
      cost = c;
      parent = node1;
      next = node2;
      prev = node3;
    }
};

#endif // __NODE_H_
