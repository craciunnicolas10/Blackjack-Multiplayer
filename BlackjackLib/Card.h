#pragma once
 
enum class ESuit
{
	Clubs,
	Diamonds,
	Hearts,
	Spades
};
 
enum class ERank
{
	Two = 2,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Ten,
	Jack,
	Queen,
	King,
	Ace
};
 

struct Card
{
	ERank rank;
	ESuit suit;
};