export function isStepForward(oldValue, newValue, length)
{
    if (newValue == 0 && oldValue == length - 1) return true;
    if (newValue == length - 1 && oldValue == 0) return false;
    return newValue > oldValue;
}

export function getNextValue(value, length)
{
  return (value + 1) % length;
}

export function getPrevValue(value, length)
{
  --value;
  if (value == -1) return length - 1;
  return value;
}

export function inRange(beginValue, value, endValue)
{
  return value >= beginValue && value <= endValue;
}

export function circularSlice(list, from, to)
{
  let newList = list.slice(from, to);
  let expectedLength = to - from;
  let remainingElements = expectedLength - newList.length;
  if (remainingElements == 0) return newList;
  newList.splice(newList.length, 0, ...list.slice(0, remainingElements));
  return newList;
}
