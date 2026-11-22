#include "AlarmSystem.h"
#include <cmath>

AlarmSystem::AlarmSystem()
{
    reset();
}

void AlarmSystem::start(int optimal)
{
    reset();

    if (optimal < 1)
        optimal = 1;

    optimalSteps = optimal;
    alarmCapacity = static_cast<int>(
        std::ceil(1.5 * optimalSteps)
        ) + 2;
}

bool AlarmSystem::addSteps(int amount)
{
    if (failed || completed || amount <= 0)
        return false;

    alarmSteps += amount;

    if (alarmSteps >= alarmCapacity)
    {
        alarmSteps = alarmCapacity;
        failed = true;
        return false;
    }

    return true;
}

bool AlarmSystem::addPenalty(int alarmCost, int scoreDeduction)
{
    if (failed || completed)
        return false;

    if (alarmCost < 0)
        alarmCost = 0;

    if (scoreDeduction < 0)
        scoreDeduction = 0;

    penaltyCount++;
    penaltyPoints += scoreDeduction;

    return addSteps(alarmCost);
}

void AlarmSystem::useHint()
{
    if (failed || completed)
        return;

    starCap = 2;
}

void AlarmSystem::addBonus(int points)
{
    if (failed || completed || points <= 0)
        return;

    bonusPoints += points;
}

void AlarmSystem::complete()
{
    if (!failed)
        completed = true;
}

void AlarmSystem::reset()
{
    optimalSteps = 0;
    alarmCapacity = 0;
    alarmSteps = 0;

    penaltyCount = 0;
    penaltyPoints = 0;
    bonusPoints = 0;

    starCap = 3;

    failed = false;
    completed = false;
}

int AlarmSystem::getOptimalSteps() const
{
    return optimalSteps;
}

int AlarmSystem::getAlarmCapacity() const
{
    return alarmCapacity;
}

int AlarmSystem::getAlarmSteps() const
{
    return alarmSteps;
}

int AlarmSystem::getRemainingAlarm() const
{
    return alarmCapacity - alarmSteps;
}

int AlarmSystem::getPenaltyCount() const
{
    return penaltyCount;
}

int AlarmSystem::getPenaltyPoints() const
{
    return penaltyPoints;
}

int AlarmSystem::getBonusPoints() const
{
    return bonusPoints;
}

int AlarmSystem::getScore() const
{
    int score = 1000 - (10 * alarmSteps)
        - penaltyPoints + bonusPoints;

    return score < 0 ? 0 : score;
}

int AlarmSystem::getStars() const
{
    if (failed || !completed)
        return 0;

    int stars = 1;

    if (alarmSteps <= optimalSteps &&
        penaltyCount == 0)
    {
        stars = 3;
    }
    else if (alarmSteps <= 1.25 * optimalSteps &&
        penaltyCount <= 1)
    {
        stars = 2;
    }

    if (stars > starCap)
        stars = starCap;

    return stars;
}

bool AlarmSystem::isFailed() const
{
    return failed;
}

bool AlarmSystem::isCompleted() const
{
    return completed;
}

bool AlarmSystem::isAlarmFull() const
{
    return alarmSteps >= alarmCapacity;
}