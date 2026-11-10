#pragma once

#include "GameEnums.h"
#include "IGameListener.h"
#include "IPlayer.h"

#include <memory>
#include <string>

class IGame
{
public:
	virtual ~IGame() = default;

	static std::shared_ptr<IGame> Produce();

	virtual void AddListener(std::weak_ptr<IGameListener> listener) = 0;
	virtual void RemoveListener(IGameListener* listener) = 0;

	virtual bool AddPlayer(const std::string& name) = 0;
	virtual bool PlaceBet(int playerIndex, int amount) = 0;
	virtual bool StartRound() = 0;

	virtual bool Hit() = 0;
	virtual bool Stand() = 0;
	virtual bool DoubleDown() = 0;

	virtual EGameState GetState() const = 0;
	virtual int GetPlayerCount() const = 0;
	virtual int GetCurrentPlayerIndex() const = 0;
	virtual const IPlayer* GetPlayer(int playerIndex) const = 0;
	virtual const IPlayer* GetDealer() const = 0;
};