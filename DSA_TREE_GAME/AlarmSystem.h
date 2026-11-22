#pragma once

class AlarmSystem
{
private:
    int optimalSteps;
    int alarmCapacity;
    int alarmSteps;

    int penaltyCount;
    int penaltyPoints;
    int bonusPoints;

    int starCap;

    bool failed;
    bool completed;

public:
    AlarmSystem();

    void start(int optimal);

    // Adds normal alarm steps (room visits, comparisons, etc.).
    bool addSteps(int amount = 1);

    // Adds a penalty, its alarm cost, and optional score deduction.
    bool addPenalty(int alarmCost = 1, int scoreDeduction = 0);

    // Using a hint limits the maximum possible stars.
    void useHint();

    void addBonus(int points);

    // Marks the mission as successfully completed.
    void complete();

    void reset();

    int getOptimalSteps() const;
    int getAlarmCapacity() const;
    int getAlarmSteps() const;
    int getRemainingAlarm() const;

    int getPenaltyCount() const;
    int getPenaltyPoints() const;
    int getBonusPoints() const;

    int getScore() const;
    int getStars() const;

    bool isFailed() const;
    bool isCompleted() const;
    bool isAlarmFull() const;
};