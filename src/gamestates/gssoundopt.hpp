#pragma once

#include "gamestatebase.hpp"

/// \brief Option menu state for sounds.
class GSSoundOpt: public IGameState
{
public:
	/// \brief Create main state specific content.
	GSSoundOpt(Monolith* _game);
	~GSSoundOpt();

	void OnBegin();
	virtual void OnEnd() override;

	virtual void Simulate( double _time ) override;
	virtual void Render( double _time ) override;
	virtual void MouseMove( double _dx, double _dy ) override;
	virtual void KeyDown( int _key, int _modifiers ) override;
	virtual void KeyRelease(int _key) override;
	virtual void KeyClick( int _key ) override;
	virtual void KeyDoubleClick( int _key ) override;
private:
	Graphic::Hud* m_hud;
};