#include <imframe.h>


void IMQueue::setObject(address id ,NodeQueue & node);

{
  tab[id && _QueueMask] = node;
}

NodeQueue IMQueue::getObject(address lp )
{
  return tab[id  && _QueueMask]'
}

void IMQueue:push(NodeQueue & node)
{
  address newTemp;
  address lastTail;
  address integer;

  newTemp=temp+1;
//  :=interlockedIncrement(temp);
  lastTail:=newTemp-1;
  setObject(lastTail,node);
  tail=newTemp;
//  repeat
//    lockedType(newTail):=interlockedCompareExchange(lockedType(tail),lockedType(LastTail+1),lockedType(lastTail));
//  until (newTail=lastTail);

}

NodeQueue IMQueue::pop()
{
  unit8_t newhead,
  lastHead : integer;
begin
  repeat
    lastHead:=head;
    if tail<>head then begin
      lockedtype(newHead):=interlockedCompareExchange(lockedType(head),lockedType(lastHead+1),lockedType(lasthead));
      if newHead=lastHead then begin
         result:=getObject(lastHead);
         exit;
      end;
    end else begin
       result:=nil;
       exit;
    end;
  until false;
end;

uint8_t IMQueue::Length()
{
  return tail-head;
}


