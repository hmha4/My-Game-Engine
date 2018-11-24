#pragma once

class Command
{
public:
	virtual ~Command() {}

	virtual void Print() const = 0;
	virtual void Execute() = 0;
	virtual class GameModel * Actor() = 0;
};