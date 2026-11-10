#pragma once

#include "Card.h"
#include "GameEnums.h"

class IGameListener
{
public:
	virtual ~IGameListener() = default;

	virtual void OnGameStateChanged(EGameState newState) = 0;
	virtual void OnPlayerAdded(int playerIndex) = 0;
	virtual void OnBetPlaced(int playerIndex, int amount) = 0;
	virtual void OnPlayerCardDealt(int playerIndex, Card card) = 0;
	virtual void OnDealerCardDealt(Card card, bool isFaceDown) = 0;
	virtual void OnDealerCardRevealed(Card card) = 0;
	virtual void OnTurnChanged(int playerIndex) = 0;
	virtual void OnRoundEnded(int playerIndex, ERoundResult result) = 0;
};