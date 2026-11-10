#pragma once

#include "Card.h"

#include <string>
#include <vector>


class IPlayer
{
public:
	virtual ~IPlayer() = default;

	virtual std::string GetName() const = 0;
	virtual std::vector<Card> GetHand() const = 0;
	virtual int GetScore() const = 0;
	virtual int GetBalance() const = 0;
	virtual int GetBet() const = 0;
};