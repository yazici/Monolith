#include "font.hpp"
#include "../content.hpp"

using namespace ei;


//#define </cm(r,g,b,a) "</c + (char)r + (char)g + (char)b + (char)a"

namespace Graphic
{
	Font::Font(std::string _fontName) :
		m_texture("texture/"+_fontName+".png"),
		m_effect( "shader/font.vs", "shader/font.ps", "shader/font.gs")
	{
		m_effect.SetRasterizerState(RasterizerState(RasterizerState::CULL_MODE::NONE, RasterizerState::FILL_MODE::SOLID));
		m_effect.SetBlendState(BlendState(BlendState::BLEND_OPERATION::ADD, BlendState::BLEND::SRC_ALPHA, BlendState::BLEND::INV_SRC_ALPHA));
		m_effect.SetDepthStencilState(DepthStencilState(DepthStencilState::COMPARISON_FUNC::ALWAYS, false));
		m_effect.BindTexture( "u_characterTex", 7, Graphic::Resources::GetSamplerState(SamplerStates::LINEAR) );
		m_effect.BindUniformBuffer( Graphic::Resources::GetUBO(UniformBuffers::GLOBAL) );

		Jo::Files::HDDFile file("texture/"+_fontName+".sraw");
		Jo::Files::MetaFileWrapper Wrap( file, Jo::Files::Format::SRAW );
		auto& PosX = Wrap.RootNode[std::string("positionX")];
		auto& PosY = Wrap.RootNode[std::string("positionY")];
		auto& sizeX = Wrap.RootNode[std::string("sizeX")];
		auto& sizeY = Wrap.RootNode[std::string("sizeY")];
		//get values
		for(int i = 0; i < 256; i++)
		{
			m_sizeTable[i] = Vec2(sizeX[i],sizeY[i]);
			// Half pixel offset necessary - otherwise rounding errors in shader
			m_coordTable[i] = Vec2(float(PosX[i]) + 0.5f/m_texture.Width(),PosY[i]);
		//	m_sizeTable[i] = Vec2(1-0.01f*i,i*0.01f);//0.1; 0.25
		//	m_coordTable[i] = Vec2(0.f,0.25f);
		}
	}

	TextRender::TextRender(Font* _font) :
		m_font(_font),
		m_characters( "222c11", VertexBuffer::PrimitiveType::POINT ),//222c11
		m_screenPos( 0.0f ),
		m_sizeD(2.f),
		m_colorD((uint8)255,(uint8)255,(uint8)255,(uint8)255),
		m_thicknessD(0.7f),
		m_active(true),
		m_sizeMax(0)
	{
		m_size = m_sizeD;
		m_color = m_colorD;
		m_thickness = m_thicknessD;

		//calculates the screen ratio
		GetDim();
	}


	void TextRender::SetText(const std::string& _text)
	{ 
		m_text = _text;
		RenewBuffer();
	}


	void TextRender::SetPos(Vec2 _screenPos)
	{ 
		m_screenPos = _screenPos;
		RenewBuffer();
//		CharacterVertex* CharVertex = (CharacterVertex*)m_characters.Get( 0 );
//		for( int i=0; i<m_text.length(); i++)
//			CharVertex[i].position += CharVertex[i].position - _screenPos; 
	}

	void TextRender::SetDefaultSize(float _size)
	{
		m_sizeD = _size;
		//only happens on control char endings, thus refresh now
		m_size = m_sizeD;
		RenewBuffer();
	}

	Vec2 TextRender::GetDim()
	{
		m_charSize = Vec2(m_font->m_sizeTable[0][0],
								   m_font->m_sizeTable[0][1] * (m_font->m_texture.Height()
								   / (float)m_font->m_texture.Width()) * Device::GetAspectRatio());
		return m_charSize;
	}


	Vec2 TextRender::GetExpanse()
	{
		return m_expanse;
	}

	void TextRender::SetExpanse(const Vec2& _expanse, bool _onlyScaleDown)
	{
		int len = (int)m_text.length();
		int lineCount = 1;
		int charCount = 0;
		int charCountMax = 0;
		for (int i = 0; i < len; i++)
		{
			charCount++;
			if (m_text[i] == '\n')
			{
				if (charCountMax < charCount)
					charCountMax = charCount;
				charCount = 0;
				lineCount++;
			}
		}

		//if the text contains no linebreaks
		if (!charCountMax) charCountMax = charCount;

		Vec2 captionDim = GetDim();

		// in case that the text is to large in any direction scale it down
		if (!_onlyScaleDown || (captionDim[0] * charCountMax * GetDefaultSize() >= _expanse[0] || captionDim[1] * lineCount >= _expanse[1]))
		{
			SetDefaultSize(min((float)(_expanse[0] / (captionDim[0] * charCountMax)),
				(float)(_expanse[1] / (captionDim[1] * lineCount))));
		}

	}

	void TextRender::Draw()
	{
		Graphic::Device::SetEffect( m_font->m_effect );
		Graphic::Device::SetTexture( m_font->m_texture, 7 );
		Graphic::Device::DrawVertices( m_characters, 0, m_characters.GetNumVertices() );
	}

	int TextRender::CntrlChr(int _i)
	{
		int jmpCount = 0;
		switch (m_text[_i+1])
		{
		case 's': m_size = (uint8)(m_text[_i+3]*100+m_text[_i+4]*10+m_text[_i+5]-208) * 12.5f / 255.f;
			jmpCount = 5;
			break;
		case 't': m_thickness =(uint8) (m_text[_i+3]*100+m_text[_i+4]*10+m_text[_i+5]-208) * 6.25f / 255.f;
			jmpCount = 5;
			break;
		case 'c': //m_color = Utils::Color32F((uint8)m_text[_i+2],(uint8)m_text[_i+3],(uint8)m_text[_i+4],(uint8)m_text[_i+5]));
			m_color = Utils::Color8U(
				(uint8)(m_text[_i+3]*100+m_text[_i+4]*10+m_text[_i+5]-208), //offset: '0'(0x30) -> 0x00 5328 % 256
				(uint8)(m_text[_i+7]*100+m_text[_i+8]*10+m_text[_i+9]-208),
				(uint8)(m_text[_i+11]*100+m_text[_i+12]*10+m_text[_i+13]-208),
				(uint8)(m_text[_i+15]*100+m_text[_i+16]*10+m_text[_i+17]-208));
			jmpCount = 17;
			break;
		case '/': switch (m_text[_i+2]) //statement closed, return to default values
			{
			case 's' : m_size = m_sizeD; 
				break;
			case 'c' : m_color = m_colorD; 
				break;
			case 't' : m_thickness = m_thicknessD; 
				break;
			};
			jmpCount = 2;
			break;
		}
		//count chars till the formating end: '>'
		while(m_text[_i+(jmpCount++)] != '>');
		return jmpCount;
	}

	void TextRender::RenewBuffer()
	{
		//reset the previous build
		m_characters.Clear();
		m_sizeMax = m_size / 2.0f;
		m_color = m_colorD;
		m_size = m_sizeD;
		Vec2 currentPos = m_screenPos;

		float maxExpanseX = currentPos[0];

		for(size_t i = 0; i<m_text.length(); i++)
		{
			CharacterVertex CV;
			CV.scale = m_size; 
			CV.size = m_font->m_sizeTable[(unsigned char)m_text[i]];
			CV.texCoord = m_font->m_coordTable[(unsigned char)m_text[i]];
			CV.position = currentPos;
			CV.thickness = m_thickness;
			CV.color = m_color.RGBA();

			//line break
			if(m_text[i] == '\n')
			{
				if (currentPos[0] > maxExpanseX) maxExpanseX = currentPos[0];
				currentPos[0] = m_screenPos[0]; 
				//m_screenRatio already contains the chars y size because its constant for every char
				currentPos[1] -= m_charSize[1] * m_sizeMax;//offset to lower line space
			}
			else if(m_text[i] == '<') { i += CntrlChr((int)i)-1; continue;} 
			else currentPos[0] += m_font->m_sizeTable[(unsigned char)m_text[i]][0]*m_size;  

 			m_characters.Add(CV);

			//save the greatest size that gets used in the text
			//after checking for cntrl chars
			if(m_size > m_sizeMax && m_text[i] != ' ') m_sizeMax = m_size;
		}
		m_characters.SetDirty();

		//when no line break happened
		if (currentPos[0] > maxExpanseX) maxExpanseX = currentPos[0];
		//calculate size using the start and end point; the points are always the lower left of the char
		m_expanse = abs((m_screenPos + Vec2(-maxExpanseX, m_charSize[1] * m_sizeMax - currentPos[1])));
	}
}; 