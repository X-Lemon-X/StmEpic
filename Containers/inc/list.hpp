#include <cstdlib>

#ifndef LIST_HPP
#define LIST_HPP

namespace LIST_EMB{

template <typename T>
class List{
private:
  struct Node{
    T data;
    Node *next;
    Node *prev;
  };
  Node *head;
  Node *tail;
  uint32_t _size;

public:

  List(){
    head = nullptr;
    tail = nullptr;
    _size = 0;
  }

  ~List(){
    Node *current = tail;
    Node *next;
    while(current != nullptr){
      next = current->next;
      free(current);
      current = next;
    }
  }

  void push_back(T data){
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == nullptr)
      return;
    new_node->data = data;
    new_node->next = nullptr;
    new_node->prev = nullptr;

    if(head == nullptr){
      head = new_node;
      tail = new_node;
    }else{
      tail->prev = new_node;
      new_node->next = tail;
      tail = new_node;
    }
    ++_size;
  }

  T get_front(){
    if(head == nullptr)
      return nullptr;
    return head->data;
  } 

  void pop_front(){
    if(head == nullptr)
      return;
    Node *temp = head;
    --_size;
    if(head == tail){
      head = nullptr;
      tail = nullptr;
    }else{
      head = head->prev;
      head->next = nullptr;
    }
    free(temp);
  }

  uint32_t size(){
    return _size;
  }

};
  
} // namespace LIST_EMB

#endif // LIST_HPP