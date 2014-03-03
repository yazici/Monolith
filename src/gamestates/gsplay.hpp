#pragma once

#include "gamestatebase.hpp"

/// \brief State for the main phase: in game.
class GSPlay: public IGameState
{
public:
	/// \brief Create main state specific content.
	GSPlay(Monolith* _game);
	~GSPlay();

	void OnBegin();
	virtual void OnEnd() override	{}

	virtual void Update( double _time, double _deltaTime ) override;
	virtual void Render( double _time, double _deltaTime ) override;
	virtual void UpdateInput() override;
	virtual void MouseMove( double _dx, double _dy ) override;
	virtual void Scroll( double _dx, double _dy ) override;
	virtual void KeyDown( int _key, int _modifiers ) override;
	virtual void KeyClick( int _key ) override;
private:
	Generators::Asteroid* m_astTest;
	Graphic::Hud* m_hud;
	Input::Camera* m_camera;
};