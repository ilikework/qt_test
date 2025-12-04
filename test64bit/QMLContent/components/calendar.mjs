import { getPrevValue, getNextValue } from "./convenience.mjs";

export function getCurrentDate() 
{
  return new Date();
}

export function getCurrentMonth() 
{
  return new Date().getMonth();
}

export function getCurrentYear() 
{
  return new Date().getFullYear();
}

function toObject(date)
{
  return {
    "date": date.getDate(),
    "month": date.getMonth(),
    "year": date.getFullYear()
  };
}

export function getAllDays(year, month, firstDayOfWeek, adjacentMonths = 2)
{
  if (adjacentMonths % 2 != 0) throw Error("adjacentMonths must be an even number.");

  let days = []
  let months = adjacentMonths / 2;
  let lastDay =  new Date(year, month + months + 1, 0);
  
  let firstDay = new Date(year, month - months, 1);
  let dayInWeek = firstDay.getDay();
  while (dayInWeek != firstDayOfWeek) {
    let dayBefore = new Date(year, month - months, -days.length);
    days.push(toObject(dayBefore));
    dayInWeek = getPrevValue(dayInWeek, 7);
  }
  if (days.length > 0) days.reverse();

  let prevDay;
  let prevDate = 1;
  while ((prevDay = new Date(year, month - months, prevDate)) <= lastDay) {
    days.push(toObject(prevDay));
    ++prevDate;
  }

  let dayInWeekLast = lastDay.getDay();
  let lastDayOfWeek = getPrevValue(firstDayOfWeek, 7);
  let daysAfterCount = 1;
  while (dayInWeekLast != lastDayOfWeek) {
    let dayAfter = new Date(year, month + months + 1, daysAfterCount);
    days.push(toObject(dayAfter));
    ++daysAfterCount;
    dayInWeekLast = getNextValue(dayInWeekLast, 7);
  }

  if (days.length % 7 != 0) throw Error ("days.length % 7 != 0");

  return days;
}

export function toWeeks(days)
{
  let weeks = [];
  for (let i = 0; i < days.length; i += 7) {
    weeks.push(days.slice(i, i + 7));
  }

  return weeks;
}

export function getWeekAfter(day)
{
  let days = [];
  let count = 1;
  while (count <= 7) {
    let newDay = new Date(day.year, day.month, day.date + count);
    days.push(toObject(newDay));
    ++count;
  }

  return days;
}

export function getWeekBefore(day)
{
  let days = [];
  let count = 7
  while (count > 0) {
    let newDay = new Date(day.year, day.month, day.date - count);
    days.push(toObject(newDay));
    --count;
  }

  return days;
}

function countMonths(monthList)
{
  let countList = [];
  let month = monthList[0];
  let count = 1;

  for (let i = 1; i < monthList.length; ++i) {
    if (month != monthList[i]) {
      countList.push(
        {
          "month": month,
          "count": count
        }
      );
      month = monthList[i];
      count = 1;
      continue;
    }
    ++count;
  }
  countList.push(
    {
      "month": month,
      "count": count
    }
  );

  return countList;
}

export function prevailingMonth(monthList)
{
  let monthsCount = countMonths(monthList);

  let elem = monthsCount[0];
  for (let i = 1; i < monthsCount.length; ++i) {
    if (monthsCount[i].count > elem.count) elem = monthsCount[i];
  }

  return elem.month;
}

export function getSelectedDayIndex(monthDays) 
{
  let todaysDate = new Date();
  todaysDate.setHours(0, 0, 0, 0);

  for (let i = 0; i < monthDays.length; ++i) {
    if (monthDays[i].getTime() == todaysDate.getTime()) {
      return i;
    }
  }

  return -1;
}
