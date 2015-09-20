#include <imqueue.h>
#include <imqueue.h>


void IMQueue::setObject(address id ,const NodeQueue & node)
{
  tab[id && _QueueMask] = node;
}

NodeQueue IMQueue::getObject(address id )
{
  return tab[id  && _QueueMask];
}

void IMQueue::push(const NodeQueue & node)
{
  address newTemp;
  address lastTail;
//  address newTail;

  newTemp=temp+1;
//  :=interlockedIncrement(temp);
  lastTail=newTemp-1;
  setObject(lastTail,node);
  tail=newTemp;
//  repeat
//    lockedType(newTail):=interlockedCompareExchange(lockedType(tail),lockedType(LastTail+1),lockedType(lastTail));
//  until (newTail=lastTail);

}

bool IMQueue::pop(NodeQueue & node)
{
  address newHead;
  address lastHead ;

  do{
    lastHead=head;
    if (tail!=head){
       newHead=lastHead+1;
//      lockedtype(newHead):=interlockedCompareExchange(lockedType(head),lockedType(lastHead+1),lockedType(lasthead));
      if (newHead==lastHead){
         node=getObject(lastHead);
         return true;
      }
    } else {
       return false;
    }
  }while (1);
}

unsigned short IMQueue::Length()
{
  return tail-head;
}


