#include <imqueue.h>


void IMQueue::setObject(address id ,const  NodeQueue & node)
{
  tab[id & _QueueMask] = node ;
//  tab[1] = node ;
}

NodeQueue IMQueue::getObject(address id )
{
  return tab[id  & _QueueMask];
}

void IMQueue::push(const NodeQueue & node)
{
  address newTemp;
  address lastTail;
//  address newTail;
//  temp++;
   newTemp= ++temp;
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
       head=lastHead+1;
       newHead=lastHead;
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


short IMQueue::ClassTest()
{
  IMQueue queue;
  IMFrame fr1;
  IMFrame fr2;
  fr1.Header.Function=11;
  queue.push(fr1);
  if ( !queue.pop(fr2))
   return 1;
  if (fr2.Header.Function!=11)
    return 2;
  if ( queue.pop(fr2))
   return 3;



}
